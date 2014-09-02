#ifndef FROKSERVER_H
#define FROKSERVER_H

// FaceServer defaults
#define DEFAULT_PORT                    (27015)
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "json.h"
#include "frokLibCommon.h"
#include "FrokAgentConnector.h"
/*
#pragma pack(push, 1)

typedef struct StructFrokRequest
{
    SOCKET      replySocket;
    unsigned    dataLength;
    char       *data;
} FaceRequest;

typedef struct StructSocketListenerData
{
    ;
} SocketListenerData;

#pragma pack(pop)

class FrokServer
{
private:
    unsigned short                      localPortNumber;
    SOCKET                              localSock;
    std::vector < FrokAgentConnector* > agents;
    std::vector < CommonThread* >       threadVecSocketListener;

public:
    FrokServer(std::vector<AgentInfo*> &agentsInfo, unsigned short localPort = DEFAULT_PORT);
    ~FrokServer();

    bool StartFrokServer();
    bool StopFrokServer();
protected:
    NetResult StartNetworkServer();
    NetResult StopNetworkServer();
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    void AcceptConnection();
    static void SocketListener(void* param);
};*/

#endif // FROKSERVER_H
