#ifndef FACESERVER_H
#define FACESERVER_H

// FaceServer defaults
#define DEFAULT_PORT                    (27015)
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "faceCommonLib.h"
#include "FaceAgentConnector.h"

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
