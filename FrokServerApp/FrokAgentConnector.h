#ifndef FROKAGENTCONNECTOR_H
#define FROKAGENTCONNECTOR_H

// FaceServer defaults
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "faceCommonLib.h"
/*
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

typedef enum FrokActivityAgentState
{
    FROK_AGENT_NOT_STARTED,
    FROK_AGENT_FREE,
    FROK_AGENT_BUSY,
    FROK_AGENT_STOPPED,
    FROK_AGENT_ERROR
} AgentState;
const char *AgentStateToString(AgentState state);
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

#pragma pack(pop)

class FrokAgentConnector
{
private:
    AgentInfo           netInfo;
    AgentState          state;
    SOCKET              agentSocket;
    CommonThread       *threadAgentListener;

public:
    FrokAgentConnector(AgentInfo &info);
    ~FrokAgentConnector();

    bool ConnectToAgent();
    bool DisconnectFromAgent();

    bool SendCommand(std::string command);
    AgentState GetAgentState();
protected:
    NetResult StartNetworkClient();
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    static void AgentListener(void *param);
private:
};
*/
#endif // FROKAGENTCONNECTOR_H
