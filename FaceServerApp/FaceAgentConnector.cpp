#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include "FaceAgentConnector.h"

pthread_mutex_t             faceAgentConnector_trace_cs;
pthread_mutex_t             faceAgentConnector_cs;

FaceAgentConnector::FaceAgentConnector(AgentInfo &info)
{
    netInfo.agentIpV4Address = info.agentIpV4Address;
    netInfo.agentPortNumber = info.agentPortNumber;
    state = FACE_AGENT_NOT_STARTED;

    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&faceAgentConnector_trace_cs, &mAttr);
    pthread_mutex_init(&faceAgentConnector_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadAgentListener = new CommonThread;
    agentSocket = INVALID_SOCKET;

    FACE_AGENT_CONNECTOR_TRACE(FaceAgentConnector, "new FaceAgentConnector");
}

FaceAgentConnector::~FaceAgentConnector()
{
    state = FACE_AGENT_NOT_STARTED;

    shutdown(agentSocket, 2);
    close(agentSocket);
    threadAgentListener->stopThread();
    delete threadAgentListener;

    pthread_mutex_destroy(&faceAgentConnector_cs);
    pthread_mutex_destroy(&faceAgentConnector_trace_cs);
    FACE_AGENT_CONNECTOR_TRACE(~FaceAgentConnector, "~FaceAgent");
}

bool FaceAgentConnector::ConnectToAgent()
{
    NetResult res;
    FACE_AGENT_CONNECTOR_TRACE(ConnectToAgent, "Calling StartNetworkClient");
    if(NET_SUCCESS != (res = StartNetworkClient()))
    {
        FACE_AGENT_CONNECTOR_TRACE(ConnectToAgent, "StartNetworkClient failed on error %x", res);
        state = FACE_AGENT_ERROR;
        return false;
    }
    FACE_AGENT_CONNECTOR_TRACE(ConnectToAgent, "StartNetworkClient succeed");

    state = FACE_AGENT_FREE;

    return true;
}

bool FaceAgentConnector::DisconnectFromAgent()
{
    if(agentSocket == INVALID_SOCKET)
    {
        FACE_AGENT_CONNECTOR_TRACE(DisconnectFromAgent, "Already disconnected");
        return true;
    }

    FACE_AGENT_CONNECTOR_TRACE(DisconnectFromAgent, "Calling shutdown agentSocket");
    if(-1 == shutdown(agentSocket, 2))
    {
        FACE_AGENT_CONNECTOR_TRACE(DisconnectFromAgent, "Failed to terminate connection with error = %s", strerror(errno));
        state = FACE_AGENT_ERROR;
        return false;
    }
    FACE_AGENT_CONNECTOR_TRACE(DisconnectFromAgent, "shutdown succeed");

    FACE_AGENT_CONNECTOR_TRACE(StopNetworkServer, "Closing socket descriptor. socket = %i", agentSocket);
    if(-1 == close(agentSocket))
    {
        FACE_AGENT_CONNECTOR_TRACE(StopNetworkServer, "Failed to close socket descriptor on error %s", strerror(errno));
        state = FACE_AGENT_ERROR;
        return false;
    }
    FACE_AGENT_CONNECTOR_TRACE(StopNetworkServer, "Descriptor successfully closed");

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
        FACE_AGENT_CONNECTOR_TRACE(SendCommand, "Agent not connected");
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

    outJson["cmd"] = "command";
    outJson["req_id"] = "0";
    outJson["reply_sock"] = -1;


    // [TBD] this is possibly incorrect, need to serialize all objects
    std::string outString = json::Serialize(outJson);

    NetResult res;
    if(NET_SUCCESS != (res = SendData(agentSocket, outString.c_str(), outString.length())))
    {
        FACE_AGENT_CONNECTOR_TRACE(SendCommand, "Failed to send command to agent on error %x", res);
        return false;
    }
    return true;
}

void FaceAgentConnector::AgentListener(void *param)
{
    FaceAgentConnector     *pThis = NULL;
    memcpy(&pThis, param, sizeof(FaceAgentConnector*));

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

    std::vector<std::string> mandatoryKeys;
    mandatoryKeys.push_back("cmd");
    mandatoryKeys.push_back("req_id");
    mandatoryKeys.push_back("reply_sock");
    mandatoryKeys.push_back("result");

    FACE_AGENT_CONNECTOR_TRACE(AgentListener, "start listening to socket %i", pThis->agentSocket);

    for(;;)
    {
        if(pThis->threadAgentListener->isStopThreadReceived())
        {
            FACE_AGENT_CONNECTOR_TRACE(AgentListener, "terminate thread sema received");
            break;
        }

        if( -1 == (dataLength = recv(pThis->agentSocket, data, sizeof(data), MSG_DONTWAIT)))
        {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // Valid errors - continue
                continue;
            }
            // unspecified error occured
            FACE_AGENT_CONNECTOR_TRACE(AgentListener, "recv failed on error %s", strerror(errno));
            FACE_AGENT_CONNECTOR_TRACE(AgentListener, "SocketListener shutdown");
            shutdown(pThis->agentSocket, 2);
            close(pThis->agentSocket);
            return;
        }

        if(dataLength == 0)
        {
            // TCP keep alive
            continue;
        }

        FACE_AGENT_CONNECTOR_TRACE(AgentListener, "Received %d bytes from the socket %u", dataLength, pThis->agentSocket);

        // Response from agent received
        json::Object responseFromAgent;
        try
        {
            responseFromAgent = ((json::Value)json::Deserialize((std::string)data)).ToObject();
        }
        catch (...)
        {
            FACE_AGENT_CONNECTOR_TRACE(AgentListener, "Failed to parse incoming JSON: %s", data);
            continue;
        }

        if (!(responseFromAgent.HasKeys(mandatoryKeys)))
        {
            FACE_AGENT_CONNECTOR_TRACE(AgentListener, "Invalid input JSON: no cmd field (%s)", data);
            continue;
        }

        SOCKET replySock = responseFromAgent["reply_sock"].ToInt();
        responseFromAgent.Erase("reply_sock");
        std::string outJson = json::Serialize(responseFromAgent);

        if(NET_SUCCESS != pThis->SendData(replySock, outJson.c_str(), outJson.size()))
        {
            FACE_AGENT_CONNECTOR_TRACE(AgentListener, "Failed to send response to remote peer %u. Reponse = %s",replySock, outJson.c_str());
            continue;
        }
    }

    FACE_AGENT_CONNECTOR_TRACE(AgentListener, "SocketListener finished");
}

NetResult FaceAgentConnector::StartNetworkClient()
{
    if(threadAgentListener->getThreadState() == COMMON_THREAD_STARTED)
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "AgentListener already started. Call StopNetwork first");
        return NET_ALREADY_STARTED;
    }

    sockaddr_in     sock_addr;

    if ((agentSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    int option = 1;

    if(0 != setsockopt(agentSocket, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(int)))
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    if(0 != setsockopt(agentSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "setsockopt failed on error %s", strerror(errno));
        shutdown(agentSocket, 2);
        close(agentSocket);
        agentSocket = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);
    if(0 != getsockopt(agentSocket, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "getsockopt failed on error %s", strerror(errno));
        shutdown(agentSocket, 2);
        close(agentSocket);
        agentSocket = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "setsockopt failed to enabled SO_REUSEADDR option");
    }

    sock_addr.sin_addr.s_addr = netInfo.agentIpV4Address;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(netInfo.agentPortNumber);

    if (0 != connect(agentSocket, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)))
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "connect failed on error %s", strerror(errno));
        shutdown(agentSocket, 2);
        close(agentSocket);
        agentSocket = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    FaceAgentConnector *pThis = this;
    FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "Starting AgentListener");

    if(!threadAgentListener->startThread((void * (*)(void*))(FaceAgentConnector::AgentListener), &pThis, sizeof(FaceAgentConnector*)))
    {
        FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "Failed to start SocketListener thread. See CommonThread logs for information");
        return NET_COMMON_THREAD_ERROR;
    }

    FACE_AGENT_CONNECTOR_TRACE(StartNetworkClient, "Network client started, socket = %d", agentSocket);

    return NET_SUCCESS;
}

NetResult FaceAgentConnector::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if (-1 == (sendlen = send(sock, pBuffer, uBufferSize, 0)))
        {
            FACE_AGENT_CONNECTOR_TRACE(SendData, "Failed to send outgoing bytes to the remote peer %u with error %s", sock, strerror(errno));
            return NET_SOCKET_ERROR;
        }

        usleep(50000);      // sleep for 1 system monitor tact
        FACE_AGENT_CONNECTOR_TRACE(SendData, "%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        FACE_AGENT_CONNECTOR_TRACE(SendData, "Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}
