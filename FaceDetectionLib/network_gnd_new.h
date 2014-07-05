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

#ifndef NETWORK_GND_NEW_H
#define NETWORK_GND_NEW_H

// include dependencies
#include "os_defines.h"
#include "commonThread.h"

#define PORT                            (27015)         // [TBD]          change for everyone
#define MAX_SOCKET_BUFF_SIZE            (8400)

#ifdef NETWORK_SYSTEM_DEBUG_PRINT_ENABLED
#define NETWORK_TRACE(__function_name__, format, ...)   \
    pthread_mutex_lock(&gnd_network_cs);                \
    printf("[GND_NETWORK->%s]: ", #__function_name__);  \
    printf(format, ##__VA_ARGS__);                      \
    printf("\n");                                       \
    pthread_mutex_unlock(&gnd_network_cs)
#else
#define NETWORK_TRACE(__function_name__, format, ...)
#endif

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
    NET_INVALID_PARAM           = 0x05,
    NET_COMMON_THREAD_ERROR     = 0x06
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

#pragma pack (push, 1)

class GNDNetwork
{
private:
    // Local port
    unsigned short          localPortNumber;
    // Local socket
    SOCKET                  localSock;
    // local server IP address. just for app use
    char                   *ipv4Addr;
    // Socket Listener's thread.
    CommonThread           *threadSockListener;
    // Accept incoming connection thread.
    CommonThread           *threadAcceptConnection;
    // Callback function for TSNetwork
    NetworkProtocolCallback protocolCallback;
public:
    GNDNetwork(NetworkProtocolCallback callback, unsigned short localPort);
    ~GNDNetwork();

    // Connect to remote side. Returns socket, that should be used in SendData
    int EstablishConnetcion(DWORD remoteIP, unsigned short remotePort);

    // Initializes network socket, binds it with local ip address and creates thread, that accepts any incoming connection
    NetResult StartNetworkServer();
    // Closes all connections if there were some, disconnects from socket.
    NetResult StopNetworkServer();
    // Sends data to the remote side's socket
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
    // Returns string with local ip Address
    char* GetLocalIpAddr();
private:
    // Accepts any incoming connection
    static void AcceptConnection(void* param);
    // Recieves any incoming information, and gives it to the upper layer
    static void SocketListener(void* param);
};

typedef struct StructProtocolCallbackData
{
    SOCKET      remoteSocket;
    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];
} ProtocolCallbacklData;

typedef struct StructAcceptConnectionData
{
    GNDNetwork *pThis;
} AcceptConnectionData;

typedef struct StructSocketListenerData
{
    GNDNetwork *pThis;
    SOCKET      listenedSocket;
} SocketListenerData;

#pragma push (pop)

#endif // NETWORK_GND_NEW_H
