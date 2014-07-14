#include <errno.h>
#include "FaceAgentConnector.h"

FaceAgentConnector::FaceAgentConnector(AgentInfo &info)
    : Network(DefaultCallback)
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
    std::string outString = json::Serialize(outJson);
    if(NET_SUCCESS != SendData(agentSocket, outString.c_str(), outString.length()))
    {
        FilePrintMessage(_FAIL("Failed to send command to agent"));
        return false;
    }
    return true;
}

void FaceAgentConnector::SocketListener(void *param)
{
    SocketListenerData     *psParam = (SocketListenerData*)param;
    FaceAgentConnector     *pThis = (FaceAgentConnector*)psParam->pThis;

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

    std::vector<std::string> mandatoryKeys;
    mandatoryKeys.push_back("cmd");
    mandatoryKeys.push_back("req_id");
    mandatoryKeys.push_back("reply_sock");
    mandatoryKeys.push_back("result");

    NETWORK_TRACE(SocketListener, "start listening to socket %i", psParam->listenedSocket);

    for(;;)
    {
        if(psParam->thread->isStopThreadReceived())
        {
            NETWORK_TRACE(SocketListener, "terminate thread sema received");
            break;
        }

        if( -1 == (dataLength = recv(psParam->listenedSocket, data, sizeof(data), MSG_DONTWAIT)))
        {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // Valid errors - continue
                continue;
            }
            // unspecified error occured
            NETWORK_TRACE(SocketListener, "recv failed on error %s", strerror(errno));
            NETWORK_TRACE(SocketListener, "SocketListener shutdown");
            shutdown(psParam->listenedSocket, 2);
            return;
        }

        if(dataLength == 0)
        {
            // TCP keep alive
            continue;
        }

        NETWORK_TRACE(SocketListener, "Received %d bytes from the socket %u", sCallbackData.dataLength, psParam->listenedSocket);

        // Response from agent received
        json::Object responseFromAgent;
        try
        {
            responseFromAgent = ((json::Value)json::Deserialize((std::string)data)).ToObject();
        }
        catch (...)
        {
            FilePrintMessage(_FAIL("Failed to parse incoming JSON: %s"), data);
            continue;
        }

        if (!(responseFromAgent.HasKeys(mandatoryKeys)))
        {
            FilePrintMessage(_FAIL("Invalid input JSON: no cmd field (%s)"), (char*)param);
            continue;
        }

        SOCKET replySock = responseFromAgent["reply_sock"].ToInt();
        responseFromAgent.Erase("reply_sock");
        std::string outJson = json::Serialize(responseFromAgent);

        if(NET_SUCCESS != pThis->SendData(replySock, outJson.c_str(), outJson.size()))
        {
            FilePrintMessage(_FAIL("Failed to send response to remote peer %u. Response = %s"), replySock, outJson.c_str());
            continue;
        }
    }

    if(pThis->callback != NULL)
    {
        pThis->callback(NET_SERVER_DISCONNECTED, psParam->listenedSocket, 0, NULL);
    }

    NETWORK_TRACE(SocketListener, "SocketListener finished");
}
