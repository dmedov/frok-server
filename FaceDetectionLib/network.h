/*******************************************************************************
*
*       Copyright 2014 Motorola Solutions, Inc. All rights reserved.
*
*       The copyright notice above does not evidence any
*       actual or intended publication of such source code.
*       The code contains Motorola Confidential Proprietary Information.
*
*
 ******************************************************************************/
#pragma once
#define MAX_SOCKET_BUFF_SIZE            (8400)

#ifdef NETWORK_SYSTEM_DEBUG_PRINT_ENABLED
#define NETWORK_TRACE(format, ...)  \
    printf(format, ##__VA_ARGS__);\
    printf("\n")
#else
#define NETWORK_TRACE(format, ...)
#endif

#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>

#define UNREFERENCED_PARAMETER(P)       (P=P)
#define INVALID_SOCKET                  (-1)
#define SOCKET                          int
#define SOCKET_ERROR                    (-1)

/*******************************************************************************
* NetResult - Return code for TSNetwork functions
*******************************************************************************/
typedef enum NetResult
{
    NET_SUCCESS                 = 0x00,
    NET_SOCKET_ERROR            = 0x01,
    NET_MEM_ALLOCATION_FAIL     = 0x02,
    NET_NO_CALLBACK             = 0x03,
    NET_UNSPECIFIED_ERROR       = 0x04,
    NET_INVALID_PARAM           = 0x05
    //...
} NetResult;

/*******************************************************************************
 * EnumNetworkEvent - Network connection status
 ******************************************************************************/
typedef enum EnumNetEvent
{
    // param:   SOCKET accepted_socket - accepted socket
    NET_SERVER_CONNECTED            = 0x00,
    // param:   SOCKET listened_sock - listened socket. Should be called disconnect function on receive(for old network)
    NET_SERVER_DISCONNECTED         = 0x01,
    // param:   SOCKET sock - socket, where information has been taken from
    //          int recvlen - size of received data
    //          char *recvbuf - pointer to received data
    NET_RECEIVED_REMOTE_DATA        = 0x02
    //...
} EnumNetEvent;

typedef void(*NetworkProtocolCallback)(unsigned Event, unsigned length, void *param);

class GNDNetwork
{
protected:
    // Remote port
    unsigned short          remotePortNumber;
    // Local port
    unsigned short          localPortNumber;
    // Local socket
    SOCKET                  localSock;
    // Socket Listener's thread.
    pthread_t               sockListenerThread;
    // Accept incoming connection thread.
    pthread_t               AcceptConnectionThread;
    // HANDLE for NetworkProtocolCallback
    sem_t                   *CallbackEventSema;
    // local server IP address. just for app use
    char*                   ipv4_addr;
    // Callback function for TSNetwork
    NetworkProtocolCallback protocolCallback;
    // Indicates that local socket is closed, and NetworkServerThread needs to be finished.
    bool                    localSockClosed;
private:
    sem_t *SocketListenerStartedEventSema;
public:
    GNDNetwork();
    ~GNDNetwork();
    // Applies callback function.
    NetResult RegisterCallback(NetworkProtocolCallback callback);
    // setter for port number variables
    void SetNetParams(unsigned short remotePort, unsigned short localPort);
    // getter for remotePortNumber variable
    unsigned short GetRemotePortNumber();
    // Deregisters callback function
    NetResult DeregisterCallback();
    // Initializes network socket, binds it with local ip address and creates thread, that accepts any incoming connection
    NetResult StartNetworkServer();
    // Closes all connections if there were some, disconnects from socket.
    NetResult StopNetworkServer();
    //NetSendData - sends data to the remote side's socket
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
    //GetLocalIpAddr - returns local ip Address
    char* GetLocalIpAddr();
    // TMP for debug and tests
    void StartSocketListener(SOCKET sock);
private:
    // Accepts any incoming connection
    static void AcceptConnection(void* Param);
    // Recieves any incoming information, and gives it to the upper layer
    static void SocketListener(void* Param);
};

typedef struct StructSocketListenerData
{
    GNDNetwork *pThis;
    SOCKET      listenedSocket;
} SocketListenerData;
