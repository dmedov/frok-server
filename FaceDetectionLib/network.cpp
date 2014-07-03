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

// Include the internal headers
#include "network.h"
pthread_mutex_t gnd_network_cs;

GNDNetwork::GNDNetwork()
{
    remotePortNumber = 0;
    localPortNumber = 0;
    localSock = INVALID_SOCKET;
    localSockClosed = true;

    CallbackEventSema = new sem_t;
    SocketListenerStartedEventSema = new sem_t;

    if (0 != sem_init(CallbackEventSema, 0, 0))
    {
        NETWORK_TRACE("Failed to init semaphore with error = %d", errno);
    }

    if (0 != sem_init(SocketListenerStartedEventSema, 0, 0))
    {
        NETWORK_TRACE("Failed to init semaphore with error = %d", errno);
    }


    char                hostname[512] = {0};
    struct hostent     *host = NULL;

    gethostname(hostname, sizeof(hostname));

    if (NULL == (host = gethostbyname(hostname)))
    {
        ipv4_addr = NULL;
        NETWORK_TRACE("Invalid host name!");
    }
    else
    {
        ipv4_addr = inet_ntoa(*(struct in_addr*)host->h_addr);
    }

    localSockClosed = false;

    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&gnd_network_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);
}

GNDNetwork::~GNDNetwork()
{
    sem_destroy(CallbackEventSema);
    sem_destroy(SocketListenerStartedEventSema);
    SocketListenerStartedEventSema = NULL;
    CallbackEventSema = NULL;
    if(AcceptConnectionThread != 0)
    {
        pthread_cancel(AcceptConnectionThread);
    }
    if(sockListenerThread != 0)
    {
        pthread_cancel(sockListenerThread);
    }
    pthread_mutex_destroy(&gnd_network_cs);
}

NetResult GNDNetwork::RegisterCallback(NetworkProtocolCallback callback)
{
    if(callback == NULL)
    {
        NETWORK_TRACE("NULL callback");
        return NET_INVALID_PARAM;
    }
    protocolCallback = callback;
    return NET_SUCCESS;
}

NetResult GNDNetwork::DeregisterCallback()
{
    protocolCallback = NULL;
    return NET_SUCCESS;
}

void GNDNetwork::SetNetParams(unsigned short remotePort, unsigned short localPort)
{
    remotePortNumber = remotePort;
    localPortNumber = localPort;
    return;
}

NetResult GNDNetwork::StopNetworkServer()
{
    localSockClosed = true;

    if(close(localSock) == SOCKET_ERROR)
    {
        NETWORK_TRACE("Failed to close local socket with error = %d", errno);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    NETWORK_TRACE("Local socket was succesfully closed. Network server is stopped");

    if(AcceptConnectionThread != 0)
    {
        pthread_cancel(this->AcceptConnectionThread);
        AcceptConnectionThread = 0;
    }

    if (sockListenerThread != 0)
    {
        pthread_cancel(this->sockListenerThread);
        sockListenerThread = 0;
    }
    return NET_SUCCESS;
}

NetResult GNDNetwork::StartNetworkServer()
{
    sockaddr_in             server;
    char                    hostname[512];
    struct hostent         *host = NULL;


    if(ipv4_addr == NULL)
    {
        // Getting ipv4 address
        gethostname(hostname, sizeof(hostname));
        host = gethostbyname(hostname);
        ipv4_addr = inet_ntoa(*(struct in_addr*)host->h_addr);
    }

    memset(&server, 0, sizeof(server));
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(localPortNumber);

    if ((localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
    {
        NETWORK_TRACE("Failed to create socket with error = %i", errno);
        close(localSock);
        NETWORK_TRACE("Network server shutdown");
        return NET_SOCKET_ERROR;
    }

    NETWORK_TRACE("Socket creation succeed");

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        NETWORK_TRACE("Failed to setsockopt with error = %i", errno);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    int optval;
    int optlen;
    getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, (socklen_t*)&optlen);
    if(optval != 0)
    {
        NETWORK_TRACE("SO_REUSEADDR option is ON!");
    }

    if (bind(localSock, (struct sockaddr*)&server, sizeof(server)) != 0)
    {
        NETWORK_TRACE("Failed to bind socket with error = %i", errno);
        close(localSock);
        NETWORK_TRACE("Network server shutdown");
        return NET_SOCKET_ERROR;
    }

    NETWORK_TRACE("Socket binding succeed");

    if (listen(localSock, SOMAXCONN) != 0)
    {
        NETWORK_TRACE("Failed to establish socket listening with error = %i", errno);
        close(localSock);
        NETWORK_TRACE("Network server shutdown");
        return NET_UNSPECIFIED_ERROR;
    }

    NETWORK_TRACE("Socket listening establishment succeed. Socket %i listens via port %i", localSock, localPortNumber);

    pthread_attr_t tAttr;
    memset(&tAttr, 0, sizeof(pthread_attr_t));

    if (0 != pthread_attr_init(&tAttr))
    {
        NETWORK_TRACE("Failed to init pthread attribute. errno = %i", errno);
        return NET_UNSPECIFIED_ERROR;
    }

    if (0 != pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED))
    {
        NETWORK_TRACE("Failed to set pthread attribute. errno = %i", errno);
        return NET_UNSPECIFIED_ERROR;
    }

    // Create thread, that would accept any incoming connection
    pthread_create(&AcceptConnectionThread, &tAttr, (void * (*)(void*))(GNDNetwork::AcceptConnection), this);

    return NET_SUCCESS;
}

void GNDNetwork::AcceptConnection(void* Param)
{
    GNDNetwork             *This = (GNDNetwork*) Param;
    sockaddr                client;
    unsigned                client_length = sizeof(client);
    char                    flag = 1;
    SOCKET                  accepted_socket;
    StructSocketListenerData socketListenerData;

    for(;;)
    {
        NETWORK_TRACE("Accepting all incoming connections for socket %i", This->localSock);

        if ((accepted_socket = accept(This->localSock, &client, &client_length)) == SOCKET_ERROR)
        {
            // accept returns SOCKET_ERROR if we will close socket.
            // If we did it manually - localSockClosed flag is set.
            // Otherwise continue accpeting connections
            if(This->localSockClosed == true)
            {
                break;
            }

            NETWORK_TRACE("Failed to accept socket with error = %d. Continue.", errno);
            continue;
        }

        NETWORK_TRACE("Socket acception succeed: %u", accepted_socket);
 
        setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, &flag, 1);
        setsockopt(accepted_socket, SOL_SOCKET, SO_REUSEADDR, &flag, 1);

        socketListenerData.listenedSocket = accepted_socket;
        socketListenerData.pThis = This;

        pthread_attr_t tAttr;
        memset(&tAttr, 0, sizeof(pthread_attr_t));

        if (0 != pthread_attr_init(&tAttr))
        {
            NETWORK_TRACE("Failed to init pthread attribute. errno = %i", errno);
            return;
        }

        if (0 != pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED))
        {
            NETWORK_TRACE("Failed to set pthread attribute. errno = %i", errno);
            return;
        }

        pthread_create(&This->sockListenerThread, &tAttr, (void * (*)(void*))(GNDNetwork::SocketListener), &socketListenerData);

        sleep(1);   //Replace this with IPC. See network_gnd:windows code
        // Send NET_SERVER_CONNECTED message via callback
        if(This->protocolCallback != NULL)
        {
            This->protocolCallback(NET_SERVER_CONNECTED, sizeof(accepted_socket), &accepted_socket);
        }
        else
        {
            NETWORK_TRACE("No callback function!");
            return;
        }
    }

    close(This->localSock);
    NETWORK_TRACE("Network server closed");
    return;
}

void GNDNetwork::SocketListener(void* Param)
{
    SocketListenerData     *sData = (SocketListenerData*)Param;
    GNDNetwork             *This = sData->pThis;
    char                    recvbuf[MAX_SOCKET_BUFF_SIZE];
    int                     recvlen = 0;
    SOCKET                  listenedSocket = sData->listenedSocket;
    BYTE                   *callbackData = NULL;

    while(This->localSockClosed == false)
    {
        memset(recvbuf, 0, sizeof(recvbuf));

        NETWORK_TRACE("SocketListener: start listening to socket %i", listenedSocket);
        if ((recvlen = recv(listenedSocket, recvbuf, sizeof(recvbuf), 0)) > 0)
        {
            NETWORK_TRACE("SocketListener: received %d bytes from the socket %u", recvlen, listenedSocket);

            if(This->protocolCallback != NULL)
            {
                pthread_mutex_lock(&gnd_network_cs);
                // Send NET_RECEIVED_REMOTE_DATA message via callback
                This->protocolCallback(NET_RECEIVED_REMOTE_DATA, recvlen, recvbuf);
                NETWORK_TRACE("SocketListener: the packet from the socket %u was successfully sent to the upper layer", listenedSocket);
                if(callbackData != NULL)
                {
                    delete []callbackData;
                    callbackData = NULL;
                }
                pthread_mutex_unlock(&gnd_network_cs);
            }
            else
            {
                NETWORK_TRACE("SocketListener: NULL callback");
            }
            recvlen = 0;
        }
        else
        {
            if (recvlen == -1)
            {
                NETWORK_TRACE("Error has occured while listening to socket %u!", listenedSocket);
                break;
            }

            NETWORK_TRACE("SocketListener: Receiving function failed for the socket %u", listenedSocket);
        }
    }

    if(This->protocolCallback != NULL)
    {
        // Send NET_SERVER_DISCONNECTED message via callback
        This->protocolCallback(NET_SERVER_DISCONNECTED, sizeof(SOCKET), &listenedSocket);
    }
    else
    {
        NETWORK_TRACE("SocketListener: NULL callback");
        return;
    }
    NETWORK_TRACE("SocketListener shutdown");
    return;
}

char* GNDNetwork::GetLocalIpAddr()
{
    return ipv4_addr;
}

HANDLE GNDNetwork::GetCallbackEvent()
{
    return CallbackEventSema;
}

NetResult GNDNetwork::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if(uBufferSize < 7)
        {
            return NET_SOCKET_ERROR;
        }

        if ((sendlen = send(sock, pBuffer, uBufferSize, 0)) == SOCKET_ERROR)
        {
            NETWORK_TRACE("Failed to send outgoing bytes to the remote peer %u with error %d", sock, errno);
            return NET_SOCKET_ERROR;
        }
        NETWORK_TRACE("%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        NETWORK_TRACE("Invalid socket!");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}

void GNDNetwork::StartSocketListener(SOCKET sock)
{
    StructSocketListenerData socketListenerData;

    socketListenerData.listenedSocket = sock;
    socketListenerData.pThis = this;

    pthread_attr_t tAttr;
    memset(&tAttr, 0, sizeof(pthread_attr_t));

    if (0 != pthread_attr_init(&tAttr))
    {
        NETWORK_TRACE("Failed to init pthread attribute. errno = %i", errno);
        return;
    }

    if (0 != pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED))
    {
        NETWORK_TRACE("Failed to set pthread attribute. errno = %i", errno);
        return;
    }

    pthread_create(&this->sockListenerThread, &tAttr, (void * (*)(void*))(GNDNetwork::SocketListener), &socketListenerData);

    sleep(1);   //[TBD] replace this with IPC. See network_gnd:windows code
    return;
}

unsigned short GNDNetwork::GetRemotePortNumber()
{
    return remotePortNumber;
}
