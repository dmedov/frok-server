#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>

#include "FaceServer.h"

std::list <FaceRequest *> requests;
sem_t                       newRequestSema;
pthread_mutex_t             faceServer_cs;
FaceServer::FaceServer(unsigned char numOfAgents, unsigned short localPort, char *photoBasePath, char*targetsFolderPath)
{
    this->photoBasePath = new char[strlen(photoBasePath) + 1];
    this->targetsFolderPath = new char[strlen(targetsFolderPath) + 1];
    strcpy(this->photoBasePath, photoBasePath);
    strcpy(this->photoBasePath, photoBasePath);

    this->numOfAgents = numOfAgents;
    this->agents = new FaceActivityAgent [numOfAgents];

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
        if(!agents[i].ConnectToAgent())
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
        if(!agents[i].DisconnectFromAgent())
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
            FilePrintMessage(DEFAULT_LOG_FILE_NAME, _FAIL("Failed to post newRequestSema on error %s"), strerror(errno));
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
        /*json::Object requestJson;
        try
        {
            requestJson = ((json::Value)json::Deserialize((std::string)req->data)).ToObject();
        }
        catch (...)
        {
            FilePrintMessage(DEFAULT_LOG_FILE_NAME, _FAIL("Failed to parse incoming JSON: %s"), req->data);
            pThis->network->SendData(req->replySocket, COMMAND_WITH_LENGTH("{ \"error\":\"bad command\" }\n\0"));
            continue;
        }

        if (!requestJson.HasKey("cmd"))
        {
            FilePrintMessage(NULL, _FAIL("Invalid input JSON: no cmd field (%s)"), (char*)param);
            pThis->network->SendData(sock, "{ \"error\":\"no cmd field\" }\n\0", strlen("{ \"error\":\"no cmd field\" }\n\0"));
            return;
        }

        // Parse cmd
        if (objInputJson["cmd"].ToString() == NET_CMD_RECOGNIZE)
        {
            if (!objInputJson.HasKey("friends"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no friends field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no friends field\" }\n\0", strlen("{ \"error\":\"no friends field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("photo_id"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                return;
            }

            ContextForRecognize *psContext = new ContextForRecognize;
            psContext->arrFrinedsList = objInputJson["friends"].ToArray();
            psContext->targetImg = objInputJson["photo_id"].ToString();
            psContext->sock = sock;

            CommonThread *threadRecongnize = new CommonThread;
            threadRecongnize->startThread((void*(*)(void*))recognizeFromModel, psContext, sizeof(ContextForRecognize));
            FilePrintMessage(NULL, _SUCC("Recognizing started..."));
            // Notice that psContext should be deleted in recognizeFromModel function!
        }
        else if (objInputJson["cmd"].ToString() == NET_CMD_GET_FACES)
        {
            if (!objInputJson.HasKey("user_id"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("photo_id"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                return;
            }

            ContextForGetFaces *psContext = new ContextForGetFaces;
            psContext->userId = objInputJson["user_id"].ToString();
            psContext->photoName = objInputJson["photo_id"].ToString();
            psContext->sock = sock;

            FilePrintMessage(NULL, _SUCC("Getting faces started..."));
            CommonThread *threadGetFaces = new CommonThread;
            threadGetFaces->startThread((void*(*)(void*))getFacesFromPhoto, psContext, sizeof(ContextForGetFaces));
            // Notice that psContext should be deleted in recognizeFromModel function!
        }
        else if (objInputJson["cmd"].ToString() == NET_CMD_SAVE_FACE)
        {
            if (!objInputJson.HasKey("user_id"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("photo_id"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("face_number"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no face_number field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no face_points field\" }\n\0", strlen("{ \"error\":\"no face_number field\" }\n\0"));
                return;
            }

            ContextForSaveFaces *psContext = new ContextForSaveFaces;
            psContext->userId = objInputJson["user_id"].ToString();
            psContext->photoName = objInputJson["photo_id"].ToString();
            psContext->faceNumber = atoi(objInputJson["face_number"].ToString().c_str());
            psContext->sock = sock;

            FilePrintMessage(NULL, _SUCC("Cut face started..."));
            CommonThread *threadSaveFaces = new CommonThread;
            threadSaveFaces->startThread((void*(*)(void*))saveFaceFromPhoto, psContext, sizeof(ContextForSaveFaces));
            // Notice that psContext should be deleted in recognizeFromModel function!
        }
        else if (objInputJson["cmd"].ToString() == NET_CMD_TRAIN)
        {
            if (!objInputJson.HasKey("ids"))
            {
                FilePrintMessage(NULL, _FAIL("Invalid input JSON: no ids field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no ids field\" }\n\0", strlen("{ \"error\":\"no ids field\" }\n\0"));
                return;
            }

            ContextForTrain *psContext = new ContextForTrain;
            psContext->arrIds = objInputJson["ids"].ToArray();
            psContext->sock = sock;

            FilePrintMessage(NULL, _SUCC("Training started..."));
            CommonThread *threadTrain = new CommonThread;
            threadTrain->startThread((void*(*)(void*))generateAndTrainBase, (void*)psContext, sizeof(ContextForTrain));

            // Notice that psContext should be deleted in recognizeFromModel function!
        }
        else
        {
            FilePrintMessage(NULL, _FAIL("Invalid input JSON: invalid cmd received (%s)"), (char*)param);
            network->SendData(sock, "{ \"error\":\"invalid cmd\" }\n\0", strlen("{ \"error\":\"invalid cmd\" }\n\0"));
            return;
        }
        break;*/
    }
}























