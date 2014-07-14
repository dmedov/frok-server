#ifndef FACEAGENTCONNECTOR_H
#define FACEAGENTCONNECTOR_H

// FaceServer defaults
#define DEFAULT_PHOTO_BASE_PATH     "/home/zda/faces/"
#define DEFAULT_TARGETS_FOLDER_PATH "/home/zda/faces/"
#define DEFAULT_PORT                27015

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"

typedef enum FaceActivityAgentState
{
    FACE_AGENT_NOT_STARTED,
    FACE_AGENT_FREE,
    FACE_AGENT_BUSY,
    FACE_AGENT_STOPPED,
    FACE_AGENT_ERROR
} AgentState;

typedef struct AgentInfo
{
    __uint32_t      agentIpV4Address;
    unsigned short  agentPortNumber;
    AgentInfo()
    {
        agentIpV4Address = 0;
        agentPortNumber = 0;
    }
    AgentInfo(__uint32_t ip, unsigned short port)
    {
        agentIpV4Address = ip;
        agentPortNumber = port;
    }
} AgentInfo;

class FaceAgentConnector
{
public:
    AgentInfo netInfo;
private:
    sem_t          *agentEnvokeSema;
    CommonThread   *agentThread;
    AgentState      state;
public:
    FaceAgentConnector(AgentInfo &info);
    ~FaceAgentConnector();

    bool ConnectToAgent();
    bool DisconnectFromAgent();

    bool SendCommand();
    AgentState GetAgentState();

private:
    /*void CommandExecuter(void *pContext);
    void Recognize(void *pContext);
    void TrainUserModel(void *pContext);
    void GetFacesFromPhoto(void *pContext);
    void AddFaceFromPhoto(void *pContext);*/
};

// Contexts
struct ContexRecognize
{
    SOCKET sock;
    std::string targetImg;
    json::Array arrFrinedsList;
};

struct ContextForTrain
{
    SOCKET sock;
    json::Array arrIds;
};

struct ContextForGetFaces
{
    SOCKET sock;
    std::string userId;
    std::string photoName;
};

struct ContextForSaveFaces
{
    SOCKET sock;
    std::string userId;
    std::string photoName;
    int faceNumber;
};

#endif // FACEAGENTCONNECTOR_H
