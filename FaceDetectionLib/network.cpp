#include "stdafx.h"

// Include the internal headers
#include "network.h"

CRITICAL_SECTION Network::network_cs;
HANDLE Network::socketListenerStartedEvent;

Network::Network()
{
    NETWORK_TRACE("Network", "Object created");
	sockListenerThread          = NULL;
	acceptConnectionThread      = NULL;
    socketListenerStartedEvent  = NULL;
	callback					= NULL;
    localPort					= 0;
	localSock                   = INVALID_SOCKET;
    isLocalSockClosed           = false;

    InitializeCriticalSection(&network_cs);
}

Network::~Network()
{
    DeleteCriticalSection(&network_cs);
    NETWORK_TRACE("~Network", "Object deleted");
}

NetResult Network::StopNetworkServer()
{
    EnterCriticalSection(&network_cs);

    // Both acceptConnectionThread and sockListenerThread will be terminated after closesocket(this->localSock);
    // First is terminated because of error in listening to local socket function. Seond is terminated because
    // of closing listened socket in acceptConnectionThread exit scenario.
    if(localSock != INVALID_SOCKET)
    {
        isLocalSockClosed = true;
        closesocket(localSock);

        if(acceptConnectionThread != NULL)
        {
            if(WAIT_OBJECT_0 != WaitForSingleObject(acceptConnectionThread, TERMINATE_NET_THREAD_TIMEOUT))
            {
                NETWORK_TRACE("StopNetworkServer", "Failed to terminate acceptConnectionThread. Continue...");
            }

            CloseHandle(acceptConnectionThread);
            acceptConnectionThread = NULL;
        }

        if(sockListenerThread != NULL)
        {
            if(WAIT_OBJECT_0 != WaitForSingleObject(sockListenerThread, TERMINATE_NET_THREAD_TIMEOUT))
            {
                NETWORK_TRACE("StopNetworkServer", "Failed to terminate sockListenerThread. Continue...");
            }
            CloseHandle(sockListenerThread);
            sockListenerThread = NULL;
        }
    }
    else
    {
        if(acceptConnectionThread != NULL)
        {
            NETWORK_TRACE("StopNetworkServer", "acceptConnectionThread should have been closed!");
            CloseHandle(acceptConnectionThread);
            acceptConnectionThread = NULL;
        }

        if(sockListenerThread != NULL)
        {
            NETWORK_TRACE("StopNetworkServer", "sockListenerThread should have been closed!");
            CloseHandle(sockListenerThread);
            sockListenerThread = NULL;
        }
    }
	
	if(socketListenerStartedEvent != NULL)
    {
        CloseHandle(socketListenerStartedEvent);
        socketListenerStartedEvent = NULL;
    }

    NETWORK_TRACE("StopNetworkServer", "Local socket was succesfully closed. Network server is stopped");

    LeaveCriticalSection(&network_cs);

    return NET_SUCCESS;
}

NetResult Network::StartNetworkServer(NetworkProtocolCallback callback, unsigned short localPortNumber)
{
    WSADATA                 wsaData;
    char                    flag = 1;

    struct addrinfo        *localAddrInfo = NULL;
    struct addrinfo         hints;
    char                    port[6];

    // Initialize local variables
    this->localPort             = localPortNumber;
    isLocalSockClosed           = false;      // Reset flag
    socketListenerStartedEvent  = CreateEvent(NULL, FALSE, FALSE, NULL);

    this->callback = callback;

    //Initialize DLL for winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData))
    {
        NETWORK_TRACE("StartNetworkServer", "Failed to initialize DLL for winsock");
        return NET_SOCKET_ERROR;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    _itoa_s(localPortNumber, port, sizeof(port), 10);

	struct hostent         *host = NULL;
	char                    hostname[512];
    // Getting ipv4 address
    gethostname(hostname, sizeof(hostname));
    host = gethostbyname(hostname);
    char* localIPv4Addr = inet_ntoa(*(struct in_addr*)host->h_addr);
    
    // getting addrinfo for socket() function
    getaddrinfo(localIPv4Addr, port, &hints, &localAddrInfo);

    if ((localSock = socket(localAddrInfo->ai_family, localAddrInfo->ai_socktype, localAddrInfo->ai_protocol)) == INVALID_SOCKET)
    {
        NETWORK_TRACE("StartNetworkServer", "Failed to create socket with error = %d", WSAGetLastError());
        closesocket(localSock);
        return NET_SOCKET_ERROR;
    }

    NETWORK_TRACE("StartNetworkServer", "Socket creation succeed");

    setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &flag, 1);

    if (bind(localSock, localAddrInfo->ai_addr, (int)localAddrInfo->ai_addrlen) == SOCKET_ERROR)
    {
        NETWORK_TRACE("StartNetworkServer", "Failed to bind socket with error = %d", WSAGetLastError());
        closesocket(localSock);
        return NET_SOCKET_ERROR;
    }

    NETWORK_TRACE("StartNetworkServer", "Socket binding succeed");
 
    if (listen(localSock, SOMAXCONN) == SOCKET_ERROR)
    {
        NETWORK_TRACE("StartNetworkServer", "Failed to establish socket listening with error = %d", WSAGetLastError());
        closesocket(localSock);
        return NET_SOCKET_ERROR;
    }

    NETWORK_TRACE("StartNetworkServer", "Socket listening establishment succeed");

    // Create thread, that would accept any incoming connection
    acceptConnectionThread = CreateThread(NULL, 0, &Network::AcceptConnection,  (void*) this, 0, NULL);

    return NET_SUCCESS;
}

DWORD WINAPI Network::AcceptConnection(void* param)
{
    Network					*This = (Network*) param;
    sockaddr                client;
    int                     clientLength = sizeof(client);
    char                    flag = 1;
    SOCKET                  acceptedSocket;
    SOCKET                  storedSocket = INVALID_SOCKET;
    SocketListenerData      socketListenerData;

    for(;;)
    {
        if ((acceptedSocket = accept(This->localSock, &client, &clientLength)) == SOCKET_ERROR)
        {
            // accept returns SOCKET_ERROR if we will close socket. 
            // If we did it manually - isLocalSockClosed flag is set.
            // Otherwise continue accpeting connections

            if(This->isLocalSockClosed == true)
            {
                // Terminating sockListenerThread
                if(storedSocket != INVALID_SOCKET)
                    closesocket(storedSocket);
                break;
            }

            NETWORK_TRACE("AcceptConnection", "Failed to accept socket with error = %d. Continue...", WSAGetLastError());
            continue;
        }

        if(storedSocket != INVALID_SOCKET)
        {
            closesocket(storedSocket);
        }

        storedSocket = acceptedSocket;

        NETWORK_TRACE("AcceptConnection", "Socket acception succeed: %u", acceptedSocket);
 
        setsockopt(acceptedSocket, IPPROTO_TCP, TCP_NODELAY, &flag, 1);
        setsockopt(acceptedSocket, SOL_SOCKET, SO_REUSEADDR, &flag, 1);

        socketListenerData.pThis = This;
        socketListenerData.listenedSocket = acceptedSocket;

        This->sockListenerThread = CreateThread(NULL, 0, &Network::SocketListener, &socketListenerData, 0, NULL);

        if(WAIT_OBJECT_0 != WaitForSingleObject(socketListenerStartedEvent, START_THREAD_TIMEOUT))
        {
            if(acceptedSocket != INVALID_SOCKET)
            {
                closesocket(acceptedSocket);
            }
            NETWORK_TRACE("AcceptConnection", "Timeout during waiting for socketListenerStartedEvent");
            return NET_UNSPECIFIED_ERROR;
        }

        // Send NET_SERVER_CONNECTED message via callback
        if(This->callback != NULL)
        {
            This->callback(acceptedSocket, NET_SERVER_CONNECTED, 0, NULL);
        }
        else
        {
            if(acceptedSocket != INVALID_SOCKET)
            {
                closesocket(acceptedSocket);
            }
            NETWORK_TRACE("AcceptConnection", "No callback function!");
            return NET_NO_CALLBACK;
        }
    }

    NETWORK_TRACE("AcceptConnection", "Network server closed");
    return NET_SUCCESS;
}

DWORD WINAPI Network::SocketListener(void* param)
{
    SocketListenerData     *sData = (SocketListenerData*)param;
    Network					*This = NULL;
    char                    recvbuf[MAX_SOCKET_BUFF_SIZE];
    int                     recvlen = 0;
    SOCKET                  listenedSocket;
    BYTE                   *callbackData = NULL;

    listenedSocket = sData->listenedSocket;
    This = sData->pThis;

    SetEvent(socketListenerStartedEvent);       // Syncronized read finished

    if(This->callback == NULL)
    {
        return NET_NO_CALLBACK;
    }

    while(This->isLocalSockClosed == false)
    {
        memset(recvbuf, 0, sizeof(recvbuf));

        if ((recvlen = recv(listenedSocket, recvbuf, sizeof(recvbuf), 0)) > 0)
        {
            NETWORK_TRACE("SocketListener", "received %d bytes from the socket %u", recvlen, listenedSocket);

            EnterCriticalSection(&network_cs);
		
            // Send NET_received_REMOTE_DATA message via callback
            This->callback(listenedSocket, NET_RECEIVED_REMOTE_DATA, recvlen, recvbuf);
            NETWORK_TRACE("SocketListener", "the packet from the socket %u was successfully sent to the upper layer", listenedSocket);

            LeaveCriticalSection(&network_cs);

            recvlen = 0;
        }
        else
        {
            if (recvlen == -1)
            {
                NETWORK_TRACE("SocketListener", "Error has occured while listening to socket %u!", listenedSocket);
                break;
            }

            NETWORK_TRACE("SocketListener", "Receiving function failed for the socket %u with error = %d", listenedSocket, WSAGetLastError());
        }
    }
        // Send NET_SERVER_DISCONNECTED message via callback
    This->callback(listenedSocket, NET_SERVER_DISCONNECTED, 0, NULL);

    NETWORK_TRACE("SocketListener", "shutdown");
    return NET_SUCCESS;
}

NetResult Network::SendData(SOCKET sock, const char* buffer, unsigned bufferSize)
{
    if(sock != INVALID_SOCKET)
    {
	    int sendlen = 0;
        if ((sendlen = send(sock, buffer, bufferSize, 0)) == SOCKET_ERROR)
        {
            NETWORK_TRACE("SendData", "Failed to send outgoing bytes to the remote peer %u with error %d", sock, WSAGetLastError());
            return NET_SOCKET_ERROR;
        }
        NETWORK_TRACE("SendData", "%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        NETWORK_TRACE("SendData", "Failed to send data: invalid socket!");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}