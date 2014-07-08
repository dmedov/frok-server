#include "stdafx.h"
#include "network.h"

pthread_mutex_t network_cs;

Network::Network(NetworkProtocolCallback callback, unsigned short localPort)
{
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&network_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadSockListener = new CommonThread;
    threadAcceptConnection = new CommonThread;
    this->localPortNumber = localPort;
    localSock = INVALID_SOCKET;
    this->protocolCallback = callback;
}

Network::~Network()
{
    close(localSock);
    threadSockListener->stopThread();
    threadAcceptConnection->stopThread();
    pthread_mutex_destroy(&network_cs);

    delete threadSockListener;
    delete threadAcceptConnection;
}

NetResult Network::StopNetworkServer()
{
    NetResult res = NET_SUCCESS;
    bool failure = false;

    NETWORK_TRACE(StopNetworkServer, "Calling socket shutdown. localSock = %i", localSock);
    if(-1 == shutdown(localSock, 2))      // 2 = Both reception and transmission
    {
        NETWORK_TRACE(StopNetworkServer, "Failed to shutdown local socket with error = %s", strerror(errno));
        failure = true;
        res = NET_SOCKET_ERROR;
    }
    /*if(close(localSock) == SOCKET_ERROR)
    {
        NETWORK_TRACE(StopNetworkServer, "Failed to close local socket with error = %s", strerror(errno));
        close(localSock);
        failure = true;
    }*/

    NETWORK_TRACE(StopNetworkServer, "Calling threadSockListener->stopThread");
    if(!threadSockListener->stopThread())
    {
        failure = true;
        res = NET_COMMON_THREAD_ERROR;
    }
    NETWORK_TRACE(StopNetworkServer, "Calling threadAcceptConnection->stopThread");
    if(!threadAcceptConnection->stopThread())
    {
        failure = true;
        res = NET_COMMON_THREAD_ERROR;
    }

    if(failure == true)
    {
        return res;
    }

    return NET_SUCCESS;
}

NetResult Network::StartNetworkServer()
{
    sockaddr_in             server;

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(localPortNumber);

    if ((localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        NETWORK_TRACE(StartNetworkServer, "socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    //[TBD] think about SO_KEEPALIVE option

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        NETWORK_TRACE(StartNetworkServer, "setsockopt failed on error %s", strerror(errno));
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);

    if(0 != getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        NETWORK_TRACE(StartNetworkServer, "getsockopt failed on error %s", strerror(errno));
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        NETWORK_TRACE(StartNetworkServer, "setsockopt failed to enabled SO_REUSEADDR option");
        //return NET_SOCKET_ERROR;
    }

    if(0 != bind(localSock, (struct sockaddr*)&server, sizeof(server)))
    {
        NETWORK_TRACE(StartNetworkServer, "bind failed on error %s", strerror(errno));
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if (0 != listen(localSock, SOMAXCONN))
    {
        NETWORK_TRACE(StartNetworkServer, "listen failed on error %s", strerror(errno));
        close(localSock);
        return NET_UNSPECIFIED_ERROR;
    }

    if(!threadAcceptConnection->startThread((void * (*)(void*))(Network::AcceptConnection), this, sizeof(Network)))
    {
        NETWORK_TRACE(StartNetworkServer, "Failed to start AcceptConnection thread. See CommonThread logs for information");
        return NET_COMMON_THREAD_ERROR;
    }

    NETWORK_TRACE(StartNetworkServer, "Succeed, socket = %d, port = %d", localSock, localPortNumber);

    return NET_SUCCESS;
}

void Network::AcceptConnection(void* param)
{
    Network             *pThis = (Network*) param;
    SOCKET                  accepted_socket;
    StructSocketListenerData socketListenerData;
    socketListenerData.pThis = pThis;
    unsigned socketListenerDataLength = sizeof(Network) + sizeof(SOCKET);

    NETWORK_TRACE(AcceptConnection, "Accepting all incoming connections for socket %i", pThis->localSock);

    for(;;)
    {
        if ((accepted_socket = accept(pThis->localSock, NULL, NULL)) == SOCKET_ERROR)
        {
            if(pThis->threadAcceptConnection->isStopThreadReceived())
            {
                NETWORK_TRACE(AcceptConnection, "terminate thread sema received");
                break;
            }
            continue;
        }

        NETWORK_TRACE(AcceptConnection, "Incoming connection accepted. Accepted socket = %u", accepted_socket);

        int flag = 1;   //TRUE
        if(0 != setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)))
        {
            NETWORK_TRACE(AcceptConnection, "setsockopt (TCP_NODELAY) failed on error %s", strerror(errno));
            continue;
        }
        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
        {
            NETWORK_TRACE(AcceptConnection, "setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            continue;
        }

        socketListenerData.listenedSocket = accepted_socket;

        if(!pThis->threadSockListener->startThread((void * (*)(void*))(Network::SocketListener), &socketListenerData, socketListenerDataLength))
        {
            NETWORK_TRACE(AcceptConnection, "Failed to start SocketListener thread. See CommonThread logs for information");
            continue;
        }

        // Send NET_SERVER_CONNECTED message via callback
        if(pThis->protocolCallback != NULL)
        {
            pThis->protocolCallback(NET_SERVER_CONNECTED, accepted_socket, 0, NULL);
        }
        else
        {
            NETWORK_TRACE(AcceptConnection, "NULL callback. AcceptConnection shutdown");
            close(pThis->localSock);
            pThis->threadSockListener->stopThread();
            return;
        }
    }

    close(pThis->localSock);
    NETWORK_TRACE(AcceptConnection, "AcceptConnection finished");
    return;
}

void Network::SocketListener(void* param)
{
    SocketListenerData     *psParam = (SocketListenerData*)param;
    Network             *pThis = psParam->pThis;

    ProtocolCallbacklData   sCallbackData;

    NETWORK_TRACE(SocketListener, "start listening to socket %i", psParam->listenedSocket);

    for(;;)
    {
        if(pThis->threadSockListener->isStopThreadReceived())
        {
            NETWORK_TRACE(SocketListener, "terminate thread sema received");
            break;
        }

        if( -1 == (sCallbackData.dataLength = recv(psParam->listenedSocket, sCallbackData.data, sizeof(sCallbackData.data), MSG_DONTWAIT)))
        {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // Valid errors - continue
                continue;
            }
            // unspecified error occured
            NETWORK_TRACE(SocketListener, "recv failed on error %s", strerror(errno));
            NETWORK_TRACE(SocketListener, "SocketListener shutdown");
            close(pThis->localSock);
            pThis->threadAcceptConnection->stopThread();
            return;
        }

        NETWORK_TRACE(SocketListener, "Received %d bytes from the socket %u", sCallbackData.dataLength, psParam->listenedSocket);

        if(pThis->protocolCallback != NULL)
        {
            pthread_mutex_lock(&network_cs);
            pThis->protocolCallback(psParam->listenedSocket, NET_RECEIVED_REMOTE_DATA, sizeof(int) + sCallbackData.dataLength, &sCallbackData);
            pthread_mutex_unlock(&network_cs);

            NETWORK_TRACE(SocketListener, "The packet from the socket %u was successfully processed by upper level callback", psParam->listenedSocket);
        }
        else
        {
            NETWORK_TRACE(SocketListener, "NULL callback. SocketListener shutdown");
            close(pThis->localSock);
            pThis->threadAcceptConnection->stopThread();
            return;
        }
    }

    if(pThis->protocolCallback != NULL)
    {
        pThis->protocolCallback(NET_SERVER_DISCONNECTED, psParam->listenedSocket, 0, NULL);
    }

    NETWORK_TRACE(SocketListener, "SocketListener finished");
    return;
}

NetResult Network::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if (-1 == (sendlen = send(sock, pBuffer, uBufferSize, 0)))
        {
            NETWORK_TRACE(SendData, "Failed to send outgoing bytes to the remote peer %u with error %s", sock, strerror(errno));
            return NET_SOCKET_ERROR;
        }
        NETWORK_TRACE(SendData, "%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        NETWORK_TRACE(SendData, "Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}

int Network::EstablishConnetcion(uint32_t remoteIPv4addr, unsigned short remotePort)
{
    UNREFERENCED_PARAMETER(remoteIPv4addr);
    UNREFERENCED_PARAMETER(remotePort);
    NETWORK_TRACE(EstablishConnetcion, "Workaround is coming...");
    return INVALID_SOCKET;
#ifdef TBD
    sockaddr_in     sock_addr;
    int connectSocket = INVALID_SOCKET;

    if (remoteIPv4addr == 0)
    {
        NETWORK_TRACE(EstablishConnetcion, "Invalid remoteIPv4addr");
        return INVALID_SOCKET;
    }

    if ((connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        NETWORK_TRACE(StartNetworkServer, "socket failed on error = %s", strerror(errno));
        return INVALID_SOCKET;
    }

    int option = 1;
    if(0 != setsockopt(connectSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        NETWORK_TRACE(StartNetworkServer, "setsockopt failed on error %s", strerror(errno));
        close(connectSocket);
        return INVALID_SOCKET;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);
    if(0 != getsockopt(connectSocket, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        NETWORK_TRACE(StartNetworkServer, "getsockopt failed on error %s", strerror(errno));
        close(connectSocket);
        return INVALID_SOCKET;
    }

    if(optval == 0)
    {
        NETWORK_TRACE(StartNetworkServer, "setsockopt failed to enabled SO_REUSEADDR option");
        //return INVALID_SOCKET;
    }

    sock_addr.sin_addr.s_addr = remoteIPv4addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(remotePort);

    if (0 != connect(connectSocket, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)))
    {
        NETWORK_TRACE(EstablishConnetcion, "connect failed on error %s", strerror(errno));
        return INVALID_SOCKET;
    }

    if(protocolCallback != NULL)
    {
        protocolCallback(NET_SERVER_CONNECTED, connectSocket, 0, NULL);
    }
    else
    {
        NETWORK_TRACE(EstablishConnetcion, "No callback. Terminating connection");
        return INVALID_SOCKET;
    }

    NETWORK_TRACE(EstablishConnetcion, "Connection successfully established");

    return connectSocket;
#endif //TBD
}
