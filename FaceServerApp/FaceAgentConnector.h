#ifndef FACEAGENTCONNECTOR_H
#define FACEAGENTCONNECTOR_H

// FaceServer defaults
#define DEFAULT_PHOTO_BASE_PATH     "/home/zda/faces/"
#define DEFAULT_TARGETS_FOLDER_PATH "/home/zda/faces/"
#define DEFAULT_PORT                27015

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"

#pragma pack(push, 1)

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

typedef enum FaceAgentCommand
{
    FACE_AGENT_RECOGNIZE,
    FACE_AGENT_TRAIN_MODEL,
    FACE_AGENT_GET_FACES_FROM_PHOTO,
    FACE_AGENT_ADD_FACE_FROM_PHOTO
} AgentCommand;

struct ParamForRecognize
{
    std::string targetImg;
    json::Array arrFrinedsList;
};

struct ParamForTrainModel
{
    json::Array arrIds;
};

struct ParamForGetFacesFromPhoto
{
    std::string userId;
    std::string photoName;
};

struct ParamForAddFaceFromPhoto
{
    std::string userId;
    std::string photoName;
    int faceNumber;
};

typedef struct FaceAgentCommandParam
{
    FaceAgentCommand cmd;
    union param
    {
        struct ParamForRecognize           *recognizeParam;
        struct ParamForTrainModel          *trainModelParam;
        struct ParamForGetFacesFromPhoto   *getFacesFromPhotoParam;
        struct ParamForAddFaceFromPhoto    *addFaceFromPhotoParam;
    };
} AgentCommandParam;

class FaceAgentConnector : public Network
{
public:
    AgentInfo netInfo;
private:
    AgentState      state;
    SOCKET          agentSocket;
public:
    FaceAgentConnector(AgentInfo &info);
    ~FaceAgentConnector();

    bool ConnectToAgent();
    bool DisconnectFromAgent();

    bool SendCommand(AgentCommandParam command);
    AgentState GetAgentState();
private:
    static void DefaultCallback(unsigned Event, SOCKET sock, unsigned length, void *param) {}
    void SocketListener(void *param);
};

#pragma pack(pop)

#endif // FACEAGENTCONNECTOR_H
