/*******************************************************************************
*
*       Copyright 2014 Motorola Solutions, Inc. All rights reserved.
*       Copyright Motorola, Inc. 2010
*
*       The copyright notice above does not evidence any
*       actual or intended publication of such source code.
*       The code contains Motorola Confidential Proprietary Information.
*
*
 ******************************************************************************/

#include "stdafx.h"
#include "os_defines.h"
#include "commonThread.h"
#include "network_gnd_new.h"

pthread_mutex_t gnd_network_cs;

GNDNetwork::GNDNetwork(NetworkProtocolCallback callback, unsigned short localPort)
{
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&gnd_network_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadSockListener = new CommonThread;
    threadAcceptConnection = new CommonThread;
    this->localPortNumber = localPort;
    localSock = INVALID_SOCKET;
    this->protocolCallback = callback;

    char                hostname[512];
    struct hostent     *host = NULL;

    memset(hostname, 0, sizeof(hostname));

    gethostname(hostname, sizeof(hostname));

    if (NULL == (host = gethostbyname(hostname)))
    {
        ipv4Addr = NULL;
        NETWORK_TRACE(GNDNetwork, "Invalid host name!");
    }
    else
    {
        ipv4Addr = inet_ntoa(*(struct in_addr*)host->h_addr);
    }
}

GNDNetwork::~GNDNetwork()
{
    close(localSock);
    threadSockListener->stopThread();
    threadAcceptConnection->stopThread();
    pthread_mutex_destroy(&gnd_network_cs);

    delete threadSockListener;
    delete threadAcceptConnection;
}

NetResult GNDNetwork::StopNetworkServer()
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

NetResult GNDNetwork::StartNetworkServer()
{
    sockaddr_in             server;
    memset(&server, 0, sizeof(server));

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(localPortNumber);

    if ((localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        NETWORK_TRACE(StartNetworkServer, "socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

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

    if(!threadAcceptConnection->startThread((void * (*)(void*))(GNDNetwork::AcceptConnection), this, sizeof(GNDNetwork*)))
    {
        NETWORK_TRACE(StartNetworkServer, "Failed to start AcceptConnection thread. See CommonThread logs for information");
        return NET_COMMON_THREAD_ERROR;
    }

    NETWORK_TRACE(StartNetworkServer, "Succeed, socket = %d, port = %d", localSock, localPortNumber);

    return NET_SUCCESS;
}

void GNDNetwork::AcceptConnection(void* param)
{
    GNDNetwork             *pThis = (GNDNetwork*) param;
    SOCKET                  accepted_socket;
    StructSocketListenerData socketListenerData;
    socketListenerData.pThis = pThis;
    unsigned socketListenerDataLength = sizeof(GNDNetwork*) + sizeof(SOCKET);

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

        if(!pThis->threadSockListener->startThread((void * (*)(void*))(GNDNetwork::SocketListener), &socketListenerData, socketListenerDataLength))
        {
            NETWORK_TRACE(AcceptConnection, "Failed to start SocketListener thread. See CommonThread logs for information");
            continue;
        }

        // Send NET_SERVER_CONNECTED message via callback
        if(pThis->protocolCallback != NULL)
        {
            pThis->protocolCallback(NET_SERVER_CONNECTED, sizeof(accepted_socket), &accepted_socket);
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

void GNDNetwork::SocketListener(void* param)
{
    SocketListenerData     *psParam = (SocketListenerData*)param;
    GNDNetwork             *pThis = psParam->pThis;

    ProtocolCallbacklData   sCallbackData;
    memset(&sCallbackData, 0, sizeof(sCallbackData));

    sCallbackData.remoteSocket = psParam->listenedSocket;

    NETWORK_TRACE(SocketListener, "start listening to socket %i", sCallbackData.remoteSocket);

    for(;;)
    {
        if(pThis->threadSockListener->isStopThreadReceived())
        {
            NETWORK_TRACE(SocketListener, "terminate thread sema received");
            break;
        }

        if( -1 == (sCallbackData.dataLength = recv(sCallbackData.remoteSocket, sCallbackData.data, sizeof(sCallbackData.data), MSG_DONTWAIT)))
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

        NETWORK_TRACE(SocketListener, "Received %d bytes from the socket %u", sCallbackData.dataLength, sCallbackData.remoteSocket);

        if(pThis->protocolCallback != NULL)
        {
            pthread_mutex_lock(&gnd_network_cs);
            pThis->protocolCallback(NET_RECEIVED_REMOTE_DATA, sizeof(SOCKET) + sizeof(int) + sCallbackData.dataLength, &sCallbackData);
            pthread_mutex_unlock(&gnd_network_cs);

            NETWORK_TRACE(SocketListener, "The packet from the socket %u was successfully processed by upper level callback", sCallbackData.remoteSocket);
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
        pThis->protocolCallback(NET_SERVER_DISCONNECTED, sizeof(SOCKET), &sCallbackData.remoteSocket);
    }

    NETWORK_TRACE(SocketListener, "SocketListener finished");
    return;
}

char* GNDNetwork::GetLocalIpAddr()
{
    return ipv4Addr;
}

NetResult GNDNetwork::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
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

int GNDNetwork::EstablishConnetcion(DWORD remoteIPv4addr, unsigned short remotePort)
{
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
        protocolCallback(NET_SERVER_CONNECTED, sizeof(SOCKET), &connectSocket);
    }
    else
    {
        NETWORK_TRACE(EstablishConnetcion, "No callback. Terminating connection");
        return INVALID_SOCKET;
    }

    NETWORK_TRACE(EstablishConnetcion, "Connection successfully established");

    return connectSocket;
}
