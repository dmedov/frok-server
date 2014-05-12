#pragma once
#pragma pointers_to_members(full_generality, multiple_inheritance)

#define PORT							(27015)
#define MAX_SOCKET_BUFF_SIZE            (8400)
#define TERMINATE_NET_THREAD_TIMEOUT    (3000)
#define START_THREAD_TIMEOUT            (500)

#ifdef NET_DEBUG_PRINT
#define NETWORK_TRACE(funcname, format, ...)  \
    printf("[NETWORK_SYSTEM_GND] %s: ", funcname);\
    printf(format, __VA_ARGS__);
    printf("\n");
#else
#define NETWORK_TRACE(funcname, format, ...)
#endif

/*******************************************************************************
* NetResult - Return code for TSNetwork functions
*******************************************************************************/
typedef enum NetworkCommand
{

    //...
} NetCmd;

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
 * NetEvent - Events that user receives in callback
 ******************************************************************************/
typedef enum NetEvent
{
    NET_SERVER_CONNECTED            = 0x00,
    NET_SERVER_DISCONNECTED         = 0x01,
    NET_RECEIVED_REMOTE_DATA        = 0x02		// see received data in *param
    //...
} NetEvent;

typedef void(*NetworkProtocolCallback)(SOCKET sock, unsigned evt, unsigned length, void *param);

class Network
{
protected:
    // Local port
    unsigned short          localPort;
    // Local socket
    SOCKET                  localSock;
    // Socket Listener's thread.
    HANDLE                  sockListenerThread;
    // Accept incoming connection thread.
    HANDLE                  acceptConnectionThread;

    // Callback function for TSNetwork
    NetworkProtocolCallback callback;
private:
    static CRITICAL_SECTION network_cs;
    static HANDLE           socketListenerStartedEvent;
    // Indicates that local socket is going to be correctly closed, and NetworkServerThread needs to be finished.
    bool                    isLocalSockClosed;
public:
    Network();
    ~Network();
    
    // Initializes network socket, binds it with local ip address and creates thread, that accepts any incoming connection
    NetResult StartNetworkServer(NetworkProtocolCallback callback, unsigned short localPortNumber = 27015);
    // Closes all connections if there were some, disconnects from socket.
    NetResult StopNetworkServer();
    
    //NetSendData - sends data to the remote side's socket
    NetResult SendData(SOCKET sock, const char* buffer, unsigned bufferSize);
private:
    // Accepts any incoming connection
    static DWORD WINAPI AcceptConnection(void* param);
    // receives any incoming information, and gives it to the upper layer
    static DWORD WINAPI SocketListener(void* param);
};

// Structure for callback listener
typedef struct StructSocketListenerData
{
    Network      *pThis;
    SOCKET          listenedSocket;
}SocketListenerData;

extern Network net;