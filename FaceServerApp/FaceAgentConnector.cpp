#include "FaceAgentConnector.h"

FaceAgentConnector::FaceAgentConnector(AgentInfo &info)
    : Network(callback)
{
    netInfo.agentIpV4Address = info.agentIpV4Address;
    netInfo.agentPortNumber = info.agentPortNumber;
    state = FACE_AGENT_NOT_STARTED;
}

FaceAgentConnector::~FaceAgentConnector()
{
    state = FACE_AGENT_NOT_STARTED;
}

bool FaceAgentConnector::ConnectToAgent()
{
    if(INVALID_SOCKET != (agentSocket = EstablishConnetcion(netInfo.agentIpV4Address, netInfo.agentPortNumber)))
    {
        FilePrintMessage(_FAIL("Failed to connect to agent"));
        state = FACE_AGENT_ERROR;
        return false;
    }

    state = FACE_AGENT_FREE;

    return true;
}

bool FaceAgentConnector::DisconnectFromAgent()
{
    if(agentSocket == INVALID_SOCKET)
    {
        FilePrintMessage(_SUCC("Already terminated"));
        return true;
    }
    if(NET_SUCCESS != TerminateConnetcion(agentSocket))
    {
        FilePrintMessage(_FAIL("Failed to disconnect from agent"));
        state = FACE_AGENT_ERROR;
        return false;
    }

    state = FACE_AGENT_STOPPED;

    return true;
}

AgentState FaceAgentConnector::GetAgentState()
{
    return state;
}

bool FaceAgentConnector::SendCommand(AgentCommandParam command)
{
    if(agentSocket == INVALID_SOCKET)
    {
        FilePrintMessage(_FAIL("Agent not connected"));
        return false;
    }
    json::Object outJson;
    switch(command.cmd)
    {
    case FACE_AGENT_RECOGNIZE:
    {
        // Make out JSON
    }
    case FACE_AGENT_TRAIN_MODEL:
    {
        // [TBD]
        break;
    }
    case FACE_AGENT_GET_FACES_FROM_PHOTO:
    {
        // [TBD]
        break;
    }
    case FACE_AGENT_ADD_FACE_FROM_PHOTO:
    {
        // [TBD]
        break;
    }
    }

    // [TBD] this is possibly incorrect, need to serialize all objects
    std::string outString = json::Serialize(json::Value(outJson));
    if(NET_SUCCESS != SendData(agentSocket, outString.c_str(), outString.length()))
    {
        FilePrintMessage(_FAIL("Failed to send command to agent"));
        return false;
    }
    return true;
}
