#ifndef NETWORK_H
#define NETWORK_H

// include dependencies

#include "commonThread.h"


// Network defaults
#define MAX_SOCKET_BUFF_SIZE            (163840)
#define MAX_CLIENTS_NUMBER              (16)
// Network defines
#define SOCKET                          int
#define SOCKET_ERROR                    (-1)
#define INVALID_SOCKET                  (-1)

// Network tracing
#ifdef NETWORK_SYSTEM_DEBUG_PRINT_ENABLED
#define NETWORK_TRACE(__function_name__, format, ...)   \
    pthread_mutex_lock(&network_trace_cs);				\
    printf("[GND_NETWORK->%s]: ", #__function_name__);  \
    printf(format, ##__VA_ARGS__);                      \
    printf("\n");                                       \
    pthread_mutex_unlock(&network_trace_cs)
#else
#define NETWORK_TRACE(__function_name__, format, ...)
#endif

// Return codes for Network class
typedef enum NetResult
{
    NET_SUCCESS                 = 0x00,
    NET_SOCKET_ERROR            = 0x01,
    NET_MEM_ALLOCATION_FAIL     = 0x02,
    NET_NO_CALLBACK             = 0x03,
    NET_UNSPECIFIED_ERROR       = 0x04,
    NET_INVALID_PARAM           = 0x05,
	NET_COMMON_THREAD_ERROR     = 0x06,
    NET_ALREADY_STARTED         = 0x07
    //...
} NetResult;

// Events for network callback
typedef enum EnumNetEvent
{
    NET_SERVER_CONNECTED            = 0x00,
    NET_SERVER_DISCONNECTED         = 0x01,
    NET_RECEIVED_REMOTE_DATA        = 0x02
} EnumNetEvent;

// Network callback prototype
typedef void(*NetworkCallback)(unsigned Event, SOCKET sock, unsigned length, void *param);

#pragma pack (push, 1)

class Network
{
private:
    // Local port
    unsigned short          localPortNumber;
    // Local socket
    SOCKET                  localSock;
    // Socket Listener's thread.
    CommonThread          **threadClientListener;
    // Accept incoming connection thread.
    CommonThread           *threadAcceptConnection;
    // Callback function for TSNetwork
    NetworkCallback callback;
public:
    Network(NetworkCallback callback, unsigned short localPort);
    Network(NetworkCallback callback);      // Only client role
    virtual ~Network();
    // Initializes network socket, binds it with local ip address and creates thread, that accepts any incoming connection
    NetResult StartNetworkServer();
    // Initializes network socket, binds it with local ip address and creates thread, that accepts any incoming connection
    NetResult StartNetworkClient(__uint32_t repoteIp, unsigned short repotePort);
    // Closes all connections if there were some, disconnects from socket.
    NetResult StopNetworkServer();
    // Sends data to the remote side's socket
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
protected:
    // Connect to remote side. Returns socket, that should be used in SendData
    SOCKET EstablishConnetcion(__uint32_t remoteIP, unsigned short remotePort);
    NetResult TerminateConnetcion(SOCKET sock);
private:
    // Accepts any incoming connection
    virtual static void AcceptConnection(void* param);
    // Recieves any incoming information, and gives it to the upper layer
    virtual static void SocketListener(void* param);
};

// Contexts for threads

typedef struct StructAcceptConnectionData
{
    Network *pThis;
} AcceptConnectionData;

typedef struct StructSocketListenerData
{

    SOCKET          listenedSocket;
    CommonThread   *thread;
    Network        *pThis;
} SocketListenerData;

#pragma pack (pop)

#endif // NETWORK_H
