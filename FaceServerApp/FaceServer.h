#ifndef FACESERVER_H
#define FACESERVER_H

// FaceServer defaults
#define DEFAULT_PORT                27015

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"
#include "FaceAgentConnector.h"

#pragma pack(push, 1)

typedef enum
{
    ROLE_AGENT,
    ROLE_CLIENT
} NetworkRole;
typedef struct
{
    SOCKET      replySocket;
    unsigned    dataLength;
    char       *data;
} FaceRequest;

class FaceServer : public Network
{
public:
private:
    std::vector<FaceAgentConnector*> agents;
public:
    FaceServer(std::vector<AgentInfo*> &agentsInfo, unsigned short localPort = DEFAULT_PORT);
    ~FaceServer();

    bool StartFaceServer();
    bool StopFaceServer();

    bool RegisterAgent(AgentInfo newAgent);
    //bool DeregisterAgent();

private:
    static void CallbackListener(void *pContext);
    static void NetworkCallback(unsigned evt, SOCKET sock, unsigned length, void *param);
    // Accepts any incoming connection
    // Recieves any incoming information, and gives it to the upper layer
    static void SocketListener(void* param);
};

#pragma pack(pop)

#endif // FACESERVER_H
