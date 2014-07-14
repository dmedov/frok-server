#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>

#include "FaceServer.h"

std::list <FaceRequest *> requests;
sem_t                       newRequestSema;
pthread_mutex_t             faceServer_cs;
FaceServer::FaceServer(std::vector<AgentInfo*> &agentsInfo, unsigned short localPort)
    : Network(FaceServer::NetworkCallback, localPort)
{
    numOfAgents = agentsInfo.size();
    if(numOfAgents != 0)
    {
        agents = new FaceAgentConnector*[numOfAgents];
        std::vector<AgentInfo*>::iterator it = agentsInfo.begin();
        for(unsigned i = 0; i < numOfAgents; i++)
        {
            agents[i] = new FaceAgentConnector(*((AgentInfo*)*it++));
        }
    }

    this->localPort = localPort;

    sem_init(&newRequestSema, 0, 0);

    network = new Network(FaceServer::NetworkCallback, this->localPort);
    threadCallbackListener = new CommonThread();
}

FaceServer::~FaceServer()
{
    delete []photoBasePath;
    delete []targetsFolderPath;
    delete network;
    delete threadCallbackListener;
    for(unsigned i = 0; i < numOfAgents; i++)
    {
        delete agents[i];
    }
    delete []agents;
}

bool FaceServer::StartFaceServer()
{
    InitFaceCommonLib();

    if(!threadCallbackListener->startThread((void*(*)(void*))FaceServer::CallbackListener, this, sizeof(FaceServer)))
    {
        return false;
    }

    if(NET_SUCCESS != network->StartNetworkServer())
    {
        return false;
    }

    for(unsigned char i = 0; i < numOfAgents; i++)
    {
        if(!agents[i]->ConnectToAgent())
        {
            return false;
        }
    }

    return true;
}

bool FaceServer::StopFaceServer()
{
    bool success = true;

    for(unsigned char i = 0; i < numOfAgents; i++)
    {
        if(!agents[i]->DisconnectFromAgent())
        {
            success = false;
        }
    }

    if(NET_SUCCESS != network->StopNetworkServer())
    {
        success = false;
    }

    if(!threadCallbackListener->stopThread())
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
    FaceServer *pThis = (FaceServer*)pContext;
    for(;;)
    {
        if(pThis->threadCallbackListener->isStopThreadReceived())
        {
            break;
        }

        if(0 != sem_trywait(&newRequestSema))
        {
            continue;
        }

        std::list<FaceRequest*>::const_iterator it = requests.begin();

        FaceRequest *req = (FaceRequest*)*it;
        req = req;
        json::Object requestJson;
        try
        {
            requestJson = ((json::Value)json::Deserialize((std::string)req->data)).ToObject();
        }
        catch (...)
        {
            FilePrintMessage(_FAIL("Failed to parse incoming JSON: %s"), req->data);
            pThis->network->SendData(req->replySocket, COMMAND_WITH_LENGTH("{ \"error\":\"bad command\" }\n\0"));
            continue;
        }

        if (!requestJson.HasKey("cmd"))
        {
            FilePrintMessage(_FAIL("Invalid input JSON: no cmd field (%s)"), (char*)param);
            pThis->network->SendData(sock, "{ \"error\":\"no cmd field\" }\n\0", strlen("{ \"error\":\"no cmd field\" }\n\0"));
            return;
        }

        // Parse cmd
        if(objInputJson["cmd"].ToString() == NET_CMD_RECOGNIZE)
        {
            if (!objInputJson.HasKey("friends"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no friends field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no friends field\" }\n\0", strlen("{ \"error\":\"no friends field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("photo_id"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                return;
            }

            ContextForRecognize *psContext = new ContextForRecognize;
            psContext->arrFrinedsList = objInputJson["friends"].ToArray();
            psContext->targetImg = objInputJson["photo_id"].ToString();
            psContext->sock = sock;

            CommonThread *threadRecongnize = new CommonThread;
            threadRecongnize->startThread((void*(*)(void*))recognizeFromModel, psContext, sizeof(ContextForRecognize));
            FilePrintMessage(_SUCC("Recognizing started..."));
            // Notice that psContext should be deleted in recognizeFromModel function!
        }

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

        if(pThis->callback != NULL)
        {
            pthread_mutex_lock(&network_cs);
            pThis->callback(psParam->listenedSocket, NET_RECEIVED_REMOTE_DATA, sizeof(int) + dataLength, data);
            pthread_mutex_unlock(&network_cs);

            NETWORK_TRACE(SocketListener, "The packet from the socket %u was successfully processed by upper level callback", psParam->listenedSocket);
        }
        else
        {
            NETWORK_TRACE(SocketListener, "NULL callback. SocketListener shutdown");
            shutdown(psParam->listenedSocket, 2);
            pThis->threadAcceptConnection->stopThread();
            return;
        }
    }

    if(pThis->callback != NULL)
    {
        pThis->callback(NET_SERVER_DISCONNECTED, psParam->listenedSocket, 0, NULL);
    }

    NETWORK_TRACE(SocketListener, "SocketListener finished");
    return;
}
