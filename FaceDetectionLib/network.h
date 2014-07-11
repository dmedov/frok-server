#ifndef NETWORK_H
#define NETWORK_H

// include dependencies
#include "commonThread.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

#define MAX_SOCKET_BUFF_SIZE            (8400)
#define UNREFERENCED_PARAMETER(P)       (P=P)
#define INVALID_SOCKET                  (-1)
#define SOCKET                          int
#define SOCKET_ERROR                    (-1)

#ifdef NETWORK_SYSTEM_DEBUG_PRINT_ENABLED
#define NETWORK_TRACE(__function_name__, format, ...)   \
    pthread_mutex_lock(&network_cs);                \
    printf("[GND_NETWORK->%s]: ", #__function_name__);  \
    printf(format, ##__VA_ARGS__);                      \
    printf("\n");                                       \
    pthread_mutex_unlock(&network_cs)
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

typedef void(*NetworkProtocolCallback)(unsigned Event, SOCKET sock, unsigned length, void *param);

#pragma pack (push, 1)

class Network
{
private:
    // Local port
    unsigned short          localPortNumber;
    // Local socket
    SOCKET                  localSock;
    // Socket Listener's thread.
    CommonThread           *threadSockListener;
    // Accept incoming connection thread.
    CommonThread           *threadAcceptConnection;
    // Callback function for TSNetwork
    NetworkProtocolCallback protocolCallback;
public:
    Network(NetworkProtocolCallback callback, unsigned short localPort);
    ~Network();

    // Connect to remote side. Returns socket, that should be used in SendData
    int EstablishConnetcion(uint32_t remoteIP, unsigned short remotePort);

    // Initializes network socket, binds it with local ip address and creates thread, that accepts any incoming connection
    NetResult StartNetworkServer();
    // Closes all connections if there were some, disconnects from socket.
    NetResult StopNetworkServer();
    // Sends data to the remote side's socket
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    // Accepts any incoming connection
    static void AcceptConnection(void* param);
    // Recieves any incoming information, and gives it to the upper layer
    static void SocketListener(void* param);
};

typedef struct StructProtocolCallbackData
{
    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];
} ProtocolCallbacklData;

typedef struct StructAcceptConnectionData
{
    Network *pThis;
} AcceptConnectionData;

typedef struct StructSocketListenerData
{
    Network *pThis;
    SOCKET      listenedSocket;
} SocketListenerData;

#pragma pack (pop)

#endif // NETWORK_H
