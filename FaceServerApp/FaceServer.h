#ifndef FACESERVER_H
#define FACESERVER_H

// FaceServer defaults
#define DEFAULT_PORT                    (27015)
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"
#include "FaceAgentConnector.h"

#ifdef FACE_SERVER_TRACE_ENABLED
#define FACE_SERVER_TRACE(__function_name__, format, ...)    \
    pthread_mutex_lock(&faceServer_trace_cs);                \
    printf("[FACE_AGENT_CONNECTOR->%s]: ", #__function_name__);       \
    printf(format, ##__VA_ARGS__);                          \
    printf("\n");                                           \
    pthread_mutex_unlock(&faceServer_trace_cs)
#else
#define FACE_SERVER_TRACE(__function_name__, format, ...)
#endif

#pragma pack(push, 1)

typedef struct StructFaceRequest
{
    SOCKET      replySocket;
    unsigned    dataLength;
    char       *data;
} FaceRequest;

typedef struct StructSocketListenerData
{

    SOCKET          listenedSocket;
    CommonThread   *thread;
    void           *pThis;
} SocketListenerData;

#pragma pack(pop)

class FaceServer
{
private:
    unsigned short                      localPortNumber;
    SOCKET                              localSock;
    std::vector < FaceAgentConnector* > agents;
    std::vector < CommonThread* >       threadVecSocketListener;
    CommonThread                       *threadAcceptConnection;

public:
    FaceServer(std::vector<AgentInfo*> &agentsInfo, unsigned short localPort = DEFAULT_PORT);
    ~FaceServer();

    bool StartFaceServer();
    bool StopFaceServer();
protected:
    NetResult StartNetworkServer();
    NetResult StopNetworkServer();
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    static void AcceptConnection(void* param);
    static void SocketListener(void* param);
};

#endif // FACESERVER_H
