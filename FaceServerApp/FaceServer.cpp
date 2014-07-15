#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>

#include "FaceServer.h"

std::list <FaceRequest *> requests;
sem_t                       newRequestSema;
pthread_mutex_t             faceServer_cs;
FaceServer::FaceServer(std::vector<AgentInfo*> &agentsInfo, unsigned short localPort)
{
    for(std::vector<AgentInfo*>::const_iterator it = agentsInfo.begin(); it != agentsInfo.end(); ++it)
    {
        agents.push_back(new FaceAgentConnector(*(AgentInfo*)*it));
    }

    //this->localPortNumber = localPort;

    sem_init(&newRequestSema, 0, 0);
}

FaceServer::~FaceServer()
{
}
/*
bool FaceServer::StartFaceServer()
{
    InitFaceCommonLib();

    for(std::vector<FaceAgentConnector*>::const_iterator it = agents.begin(); it != agents.end(); ++it)
    {
        FaceAgentConnector *agent = (FaceAgentConnector*)*it;
        if(!agent->ConnectToAgent())
        {
            FilePrintMessage(_FAIL("Failed to connect to agent"));
        }
    }
    if(NET_SUCCESS != StartNetworkServer())
    {
        return false;
    }
    return true;
}

bool FaceServer::StopFaceServer()
{
    bool success = true;

    for(std::vector<FaceAgentConnector*>::const_iterator it = agents.begin(); it != agents.end(); ++it)
    {
        FaceAgentConnector *agent = (FaceAgentConnector*)*it;
        if(!agent->DisconnectFromAgent())
        {
            success = false;
        }
    }

    if(NET_SUCCESS != StopNetworkServer())
    {
        success = false;
    }

    DeinitFaceCommonLib();

    return success;
}

void FaceServer::NetworkCallback(unsigned evt, SOCKET sock, unsigned length, void *param)
{
    if(evt == NET_RECEIVED_REMOTE_DATA)
    {
        FaceRequest *req = new FaceRequest;
        req->replySocket = sock;
        req->dataLength = length;
        req->data = new char[req->dataLength];
        memcpy(req->data, param, length);

        requests.push_back(req);

        if(-1 == sem_post(&newRequestSema))
        {
            FilePrintMessage(_FAIL("Failed to post newRequestSema on error %s"), strerror(errno));
        }
    }
}
void FaceServer::CallbackListener(void *pContext)
{
    SocketListenerData     *psParam = (SocketListenerData*)pContext;
    FaceServer *pThis = (FaceServer*)psParam->pThis;
    for(;;)
    {
        if(psParam->thread->isStopThreadReceived())
        {
            break;
        }

        if(0 != sem_trywait(&newRequestSema))
        {
            continue;
        }

        std::list<FaceRequest*>::const_iterator it = requests.begin();

        FaceRequest *req = (FaceRequest*)*it;
        json::Object requestJson;
        try
        {
            requestJson = ((json::Value)json::Deserialize((std::string)req->data)).ToObject();
        }
        catch (...)
        {
            FilePrintMessage(_FAIL("Failed to parse incoming JSON: %s"), req->data);
            pThis->SendData(req->replySocket, COMMAND_WITH_LENGTH("{ \"error\":\"bad command\" }\n\0"));
            continue;
        }

        std::vector<std::string> mandatoryFileds;
        mandatoryFileds.push_back("cmd");
        mandatoryFileds.push_back("req_id");

        //requestJson["reply_sock"] = itoa(req->replySocket);


        break;
    }
}

void FaceServer::SocketListener(void* param)
{
    SocketListenerData     *psParam = (SocketListenerData*)param;
    Network             *pThis = psParam->pThis;

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

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
    }

    NETWORK_TRACE(SocketListener, "SocketListener finished");
    return;
}
*/
