#ifndef FACEAGENTCONNECTOR_H
#define FACEAGENTCONNECTOR_H

// FaceServer defaults
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "faceCommonLib.h"

// FaceAgentConnector logging system
#ifdef FACE_AGENT_CONNECTOR_TRACE_ENABLED
#define FACE_AGENT_CONNECTOR_TRACE(__function_name__, format, ...)    \
    pthread_mutex_lock(&faceAgentConnector_trace_cs);                \
    printf("[FACE_AGENT_CONNECTOR->%s]: ", #__function_name__);       \
    printf(format, ##__VA_ARGS__);                          \
    printf("\n");                                           \
    pthread_mutex_unlock(&faceAgentConnector_trace_cs)
#else
#define FACE_AGENT_CONNECTOR_TRACE(__function_name__, format, ...)
#endif

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

typedef enum FaceActivityAgentState
{
    FACE_AGENT_NOT_STARTED,
    FACE_AGENT_FREE,
    FACE_AGENT_BUSY,
    FACE_AGENT_STOPPED,
    FACE_AGENT_ERROR
} AgentState;

#pragma pack(push, 1)

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

#pragma pack(pop)

class FaceAgentConnector
{
private:
    AgentInfo           netInfo;
    AgentState          state;
    SOCKET              agentSocket;
    CommonThread       *threadAgentListener;

public:
    FaceAgentConnector(AgentInfo &info);
    ~FaceAgentConnector();

    bool ConnectToAgent();
    bool DisconnectFromAgent();

    bool SendCommand(AgentCommandParam command);
    AgentState GetAgentState();
protected:
    NetResult StartNetworkClient();
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    static void AgentListener(void *param);
private:
};

#endif // FACEAGENTCONNECTOR_H
