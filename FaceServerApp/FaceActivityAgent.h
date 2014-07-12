#ifndef FACEACTIVITYAGENT_H
#define FACEACTIVITYAGENT_H

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

class FaceActivityAgent
{
public:
private:
    sem_t          *agentEnvokeSema;
    CommonThread   *agentThread;
    AgentState      state;
public:
    FaceActivityAgent();
    ~FaceActivityAgent();

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

#endif // FACEACTIVITYAGENT_H
