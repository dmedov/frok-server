#include "FaceAgentConnector.h"

FaceAgentConnector::FaceAgentConnector(AgentInfo &info)
{
    netInfo.agentIpV4Address = info.agentIpV4Address;
    netInfo.agentPortNumber = info.agentPortNumber;
}

FaceAgentConnector::~FaceAgentConnector()
{

}

bool FaceAgentConnector::ConnectToAgent()
{
    return true;
}

bool FaceAgentConnector::DisconnectFromAgent()
{
    return true;
}
