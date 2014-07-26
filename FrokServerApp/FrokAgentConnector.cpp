#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include "FrokAgentConnector.h"

#define MODULE_NAME     "AGENT_CONNECTOR"

pthread_mutex_t             FrokAgentConnector_trace_cs;
pthread_mutex_t             FrokAgentConnector_cs;

FrokAgentConnector::FrokAgentConnector(AgentInfo &info)
{
    netInfo.agentIpV4Address = info.agentIpV4Address;
    netInfo.agentPortNumber = info.agentPortNumber;
    state = FROK_AGENT_NOT_STARTED;

    pthread_mutexattr_t mAttr;
    pthread_mutexattr_init(&mAttr);
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&FrokAgentConnector_trace_cs, &mAttr);
    pthread_mutex_init(&FrokAgentConnector_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadAgentListener = new CommonThread;
    agentSocket = INVALID_SOCKET;

    TRACE("new FrokAgentConnector");
}

FrokAgentConnector::~FrokAgentConnector()
{
    state = FROK_AGENT_NOT_STARTED;

    shutdown(agentSocket, 2);
    close(agentSocket);
    threadAgentListener->stopThread();
    delete threadAgentListener;

    pthread_mutex_destroy(&FrokAgentConnector_cs);
    pthread_mutex_destroy(&FrokAgentConnector_trace_cs);
    TRACE("~FrokAgentConnector");
}

bool FrokAgentConnector::ConnectToAgent()
{
    NetResult res;
    TRACE("Calling StartNetworkClient");
    if(NET_SUCCESS != (res = StartNetworkClient()))
    {
        TRACE_F("StartNetworkClient failed on error %x", res);
        state = FROK_AGENT_ERROR;
        return false;
    }
    TRACE_S("StartNetworkClient succeed");

    state = FROK_AGENT_FREE;

    return true;
}

bool FrokAgentConnector::DisconnectFromAgent()
{
    if(agentSocket == INVALID_SOCKET)
    {
        TRACE_S("Already disconnected");
        return true;
    }

    TRACE("Calling shutdown agentSocket");
    if(-1 == shutdown(agentSocket, 2))
    {
        TRACE_F("Failed to terminate connection with error = %s", strerror(errno));
        state = FROK_AGENT_ERROR;
        return false;
    }
    TRACE_S("shutdown succeed");

    TRACE("Closing socket descriptor. socket = %i", agentSocket);
    if(-1 == close(agentSocket))
    {
        TRACE_F("Failed to close socket descriptor on error %s", strerror(errno));
        state = FROK_AGENT_ERROR;
        return false;
    }
    TRACE_S("Descriptor successfully closed");

    state = FROK_AGENT_STOPPED;

    return true;
}

AgentState FrokAgentConnector::GetAgentState()
{
    return state;
}

bool FrokAgentConnector::SendCommand(AgentCommandParam command)
{
    if(agentSocket == INVALID_SOCKET)
    {
        TRACE_F("Agent not connected");
        return false;
    }
    json::Object outJson;
    /*switch(command.cmd)
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
    }*/

    outJson["cmd"] = "command";
    outJson["req_id"] = "0";
    outJson["reply_sock"] = -1;


    // [TBD] this is possibly incorrect, need to serialize all objects
    std::string outString = json::Serialize(outJson);

    NetResult res;
    if(NET_SUCCESS != (res = SendData(agentSocket, outString.c_str(), outString.length())))
    {
        TRACE_F("Failed to send command to agent on error %x", res);
        return false;
    }
    return true;
}

void FrokAgentConnector::AgentListener(void *param)
{
    FrokAgentConnector     *pThis = NULL;
    memcpy(&pThis, param, sizeof(FrokAgentConnector*));

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

    std::vector<std::string> mandatoryKeys;
    mandatoryKeys.push_back("cmd");
    mandatoryKeys.push_back("req_id");
    mandatoryKeys.push_back("reply_sock");
    mandatoryKeys.push_back("result");

    TRACE("start listening to socket %i", pThis->agentSocket);

    for(;;)
    {
        if(pThis->threadAgentListener->isStopThreadReceived())
        {
            TRACE("terminate thread sema received");
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
            TRACE_F("recv failed on error %s", strerror(errno));
            TRACE_W("SocketListener shutdown");
            shutdown(pThis->agentSocket, 2);
            close(pThis->agentSocket);
            return;
        }

        if(dataLength == 0)
        {
            TRACE("Half disconnection observed. Terminating...");
            break;
        }

        TRACE("Received %d bytes from the socket %u", dataLength, pThis->agentSocket);

        // Response from agent received
        json::Object responseFromAgent;
        try
        {
            responseFromAgent = ((json::Value)json::Deserialize((std::string)data)).ToObject();
        }
        catch (...)
        {
            TRACE_F("Failed to parse incoming JSON: %s", data);
            continue;
        }

        if (!(responseFromAgent.HasKeys(mandatoryKeys)))
        {
            TRACE_F("Invalid input JSON: no cmd field (%s)", data);
            continue;
        }

        SOCKET replySock = responseFromAgent["reply_sock"].ToInt();
        responseFromAgent.Erase("reply_sock");
        std::string outJson = json::Serialize(responseFromAgent);

        if(NET_SUCCESS != pThis->SendData(replySock, outJson.c_str(), outJson.size()))
        {
            TRACE_F("Failed to send response to remote peer %u. Reponse = %s",replySock, outJson.c_str());
            continue;
        }
    }

    if(pThis->agentSocket != INVALID_SOCKET)
    {
        shutdown(pThis->agentSocket, 2);
        close(pThis->agentSocket);
    }

    TRACE("SocketListener finished");
}

NetResult FrokAgentConnector::StartNetworkClient()
{
    if(threadAgentListener->getThreadState() == COMMON_THREAD_STARTED)
    {
        TRACE_W("AgentListener already started. Call StopNetwork first");
        return NET_ALREADY_STARTED;
    }

    sockaddr_in     sock_addr;

    if ((agentSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        TRACE_F("socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    int option = 1;

    if(0 != setsockopt(agentSocket, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(int)))
    {
        TRACE_F("setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
        shutdown(agentSocket, 2);
        close(agentSocket);
        return NET_SOCKET_ERROR;
    }

    if(0 != setsockopt(agentSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        TRACE_F("setsockopt failed on error %s", strerror(errno));
        shutdown(agentSocket, 2);
        close(agentSocket);
        agentSocket = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);
    if(0 != getsockopt(agentSocket, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        TRACE_F("getsockopt failed on error %s", strerror(errno));
        shutdown(agentSocket, 2);
        close(agentSocket);
        agentSocket = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        TRACE_F("setsockopt failed to enabled SO_REUSEADDR option");
    }

    sock_addr.sin_addr.s_addr = netInfo.agentIpV4Address;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(netInfo.agentPortNumber);

    if (0 != connect(agentSocket, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)))
    {
        TRACE_F("connect failed on error %s", strerror(errno));
        shutdown(agentSocket, 2);
        close(agentSocket);
        agentSocket = INVALID_SOCKET;
        return NET_SOCKET_ERROR;
    }

    FrokAgentConnector *pThis = this;
    TRACE("Starting AgentListener");

    if(!threadAgentListener->startThread((void * (*)(void*))(FrokAgentConnector::AgentListener), &pThis, sizeof(FrokAgentConnector*)))
    {
        TRACE_F("Failed to start SocketListener thread. See CommonThread logs for information");
        shutdown(agentSocket, 2);
        close(agentSocket);
        return NET_COMMON_THREAD_ERROR;
    }

    TRACE_S("Network client started, socket = %d", agentSocket);

    return NET_SUCCESS;
}

NetResult FrokAgentConnector::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if (-1 == (sendlen = send(sock, pBuffer, uBufferSize, 0)))
        {
            TRACE_F("Failed to send outgoing bytes to the remote peer %u with error %s", sock, strerror(errno));
            return NET_SOCKET_ERROR;
        }

        usleep(50000);      // sleep for 1 system monitor tact
        TRACE("%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        TRACE_F("Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}
