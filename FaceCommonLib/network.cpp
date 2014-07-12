#include <arpa/inet.h>
#include "errno.h"
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network.h"


pthread_mutex_t network_cs;
pthread_mutex_t network_trace_cs;

Network::Network(NetworkCallback callback, unsigned short localPort)
{
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&network_cs, &mAttr);
	pthread_mutex_init(&network_trace_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadClientListener = new CommonThread* [MAX_CLIENTS_NUMBER];
    for(unsigned char i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
        threadClientListener[i] = new CommonThread;
    }
    threadAcceptConnection = new CommonThread;
    this->localPortNumber = localPort;
    localSock = INVALID_SOCKET;
    this->callback = callback;
}

Network::~Network()
{
    shutdown(localSock, 2);
    for(unsigned char i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
        threadClientListener[i]->stopThread();
        delete threadClientListener[i];
    }
    delete []threadClientListener;
    threadAcceptConnection->stopThread();
    delete threadAcceptConnection;

    pthread_mutex_destroy(&network_cs);
	pthread_mutex_destroy(&network_trace_cs);
}

NetResult Network::StopNetworkServer()
{
    NetResult res = NET_SUCCESS;

    NETWORK_TRACE(StopNetworkServer, "Calling socket shutdown. localSock = %i", localSock);
    if(-1 == shutdown(localSock, 2))      // 2 = Both reception and transmission
    {
        NETWORK_TRACE(StopNetworkServer, "Failed to shutdown local socket with error = %s", strerror(errno));
        res = NET_SOCKET_ERROR;
    }

    NETWORK_TRACE(StopNetworkServer, "Calling threadSockListener->stopThread");
    for(unsigned char i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
        if(!threadClientListener[i]->stopThread())
        {
            res = NET_COMMON_THREAD_ERROR;
        }
    }
    NETWORK_TRACE(StopNetworkServer, "Calling threadAcceptConnection->stopThread");
    if(!threadAcceptConnection->stopThread())
    {
        res = NET_COMMON_THREAD_ERROR;
    }

    return res;
}

NetResult Network::StartNetworkServer()
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

    //[TBD] think about SO_KEEPALIVE option

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        NETWORK_TRACE(StartNetworkServer, "setsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);

    if(0 != getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        NETWORK_TRACE(StartNetworkServer, "getsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
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
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    if (0 != listen(localSock, SOMAXCONN))
    {
        NETWORK_TRACE(StartNetworkServer, "listen failed on error %s", strerror(errno));
        shutdown(localSock, 2);
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

NetResult Network::StartNetworkClient(uint32_t repoteIp, unsigned short repotePort)
{
    if(threadAcceptConnection->getThreadState() == COMMON_THREAD_STARTED)
    {
        NETWORK_TRACE(StartNetworkClient, "GND network already started. You can't tstart both network client and server with same GNDNetwork unit");
        return NET_ALREADY_STARTED;
    }

    NetResult res;
    if(NET_SUCCESS != (res = EstablishConnetcion(repoteIp, repotePort)))
    {
        NETWORK_TRACE(StartNetworkClient, "Establish connection failed on error %x", res);
        return NET_SOCKET_ERROR;
    }
    SocketListenerData sData;
    sData.listenedSocket = localSock;
    sData.pThis = this;

    unsigned char i = 0;
    for(i = 0; i < MAX_CLIENTS_NUMBER; i++)
    {
        if((threadClientListener[i]->getThreadState() != COMMON_THREAD_INITIATED) && (threadClientListener[i]->getThreadState() != COMMON_THREAD_STARTED))
        {
            threadClientListener[i]->stopThread();
            sData.thread = threadClientListener[i];
            if(!threadClientListener[i]->startThread((void * (*)(void*))(Network::SocketListener), &sData, sizeof(SOCKET) + sizeof(Network) + sizeof(CommonThread*)))
            {
                NETWORK_TRACE(StartNetworkClient, "Failed to start SocketListener thread. See CommonThread logs for information");
                continue;
            }
            break;
        }
    }

    if(i == MAX_CLIENTS_NUMBER)
    {
        NETWORK_TRACE(StartNetworkClient, "MAX_CLIENTS_NUMBER reached. Terminate any connection...");
        return NET_UNSPECIFIED_ERROR;
    }

    NETWORK_TRACE(StartNetworkClient, "Network client started, socket = %d", localSock);

    return NET_SUCCESS;
}

void Network::AcceptConnection(void* param)
{
    Network             *pThis = (Network*) param;
    SOCKET                  accepted_socket;
    StructSocketListenerData socketListenerData;
    socketListenerData.pThis = pThis;
    unsigned socketListenerDataLength = sizeof(Network) + sizeof(SOCKET) + sizeof(CommonThread*);

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

        unsigned char i = 0;
        for(i = 0; i < MAX_CLIENTS_NUMBER; i++)
        {
            if((pThis->threadClientListener[i]->getThreadState() != COMMON_THREAD_INITIATED) && (pThis->threadClientListener[i]->getThreadState() != COMMON_THREAD_STARTED))
            {
                pThis->threadClientListener[i]->stopThread();
                socketListenerData.thread = pThis->threadClientListener[i];
                if(!pThis->threadClientListener[i]->startThread((void * (*)(void*))(Network::SocketListener), &socketListenerData, socketListenerDataLength))
                {
                    NETWORK_TRACE(AcceptConnection, "Failed to start SocketListener thread. See CommonThread logs for information");
                    continue;
                }
                break;
            }
        }

        if(i == MAX_CLIENTS_NUMBER)
        {
            NETWORK_TRACE(AcceptConnection, "MAX_CLIENTS_NUMBER reached. Terminate any connection...");
            shutdown(accepted_socket, 2);
        }
        else
        {
            // Send NET_SERVER_CONNECTED message via callback
            if(pThis->callback != NULL)
            {
                pThis->callback(NET_SERVER_CONNECTED, accepted_socket, 0, NULL);
            }
            else
            {
                NETWORK_TRACE(AcceptConnection, "NULL callback. AcceptConnection shutdown");
                shutdown(pThis->localSock, 2);
                return;
            }
        }
    }

    shutdown(pThis->localSock, 2);
    NETWORK_TRACE(AcceptConnection, "AcceptConnection finished");
    return;
}

void Network::SocketListener(void* param)
{
    SocketListenerData     *psParam = (SocketListenerData*)param;
    Network             *pThis = psParam->pThis;

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

    NETWORK_TRACE(SocketListener, "start listening to socket %i", psParam->listenedSocket);

    for(;;)
    {
        if(psParam->thread->isStopThreadReceived())
        {
            NETWORK_TRACE(SocketListener, "terminate thread sema received");
            break;
        }

        if( -1 == (dataLength = recv(psParam->listenedSocket, data, sizeof(data), MSG_DONTWAIT)))
        {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // Valid errors - continue
                continue;
            }
            // unspecified error occured
            NETWORK_TRACE(SocketListener, "recv failed on error %s", strerror(errno));
            NETWORK_TRACE(SocketListener, "SocketListener shutdown");
            shutdown(psParam->listenedSocket, 2);
            return;
        }

        if(dataLength == 0)
        {
            // TCP keep alive
            continue;
        }

        NETWORK_TRACE(SocketListener, "Received %d bytes from the socket %u", sCallbackData.dataLength, psParam->listenedSocket);

        if(pThis->callback != NULL)
        {
            pthread_mutex_lock(&network_cs);
            pThis->callback(psParam->listenedSocket, NET_RECEIVED_REMOTE_DATA, sizeof(int) + dataLength, data);
            pthread_mutex_unlock(&network_cs);

            NETWORK_TRACE(SocketListener, "The packet from the socket %u was successfully processed by upper level callback", psParam->listenedSocket);
        }
        else
        {
            NETWORK_TRACE(SocketListener, "NULL callback. SocketListener shutdown");
            shutdown(pThis->localSock, 2);
            pThis->threadAcceptConnection->stopThread();
            return;
        }
    }

    if(pThis->callback != NULL)
    {
        pThis->callback(NET_SERVER_DISCONNECTED, psParam->listenedSocket, 0, NULL);
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

        usleep(50000);      // sleep for 1 system monitor tact
        NETWORK_TRACE(SendData, "%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        NETWORK_TRACE(SendData, "Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}

NetResult Network::EstablishConnetcion(uint32_t remoteIPv4addr, unsigned short remotePort)
{
    sockaddr_in     sock_addr;

    if (remoteIPv4addr == 0)
    {
        NETWORK_TRACE(EstablishConnetcion, "Invalid remoteIPv4addr");
        return NET_INVALID_PARAM;
    }

    if ((localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        NETWORK_TRACE(StartNetworkServer, "socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        NETWORK_TRACE(StartNetworkServer, "setsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        localSock = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);
    if(0 != getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        NETWORK_TRACE(StartNetworkServer, "getsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        localSock = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        NETWORK_TRACE(StartNetworkServer, "setsockopt failed to enabled SO_REUSEADDR option");
        //return INVALID_SOCKET;
    }

    sock_addr.sin_addr.s_addr = remoteIPv4addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(remotePort);

    if (0 != connect(localSock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)))
    {
        NETWORK_TRACE(EstablishConnetcion, "connect failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        localSock = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    if(callback != NULL)
    {
        callback(NET_SERVER_CONNECTED, localSock, 0, NULL);
    }
    else
    {
        NETWORK_TRACE(EstablishConnetcion, "No callback. Terminating connection");
        shutdown(localSock, 2);
        localSock = INVALID_SOCKET;
        return NET_NO_CALLBACK;
    }

    NETWORK_TRACE(EstablishConnetcion, "Connection successfully established");

    return NET_SUCCESS;
}
