#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "FaceAgent.h"

pthread_mutex_t             faceAgent_trace_cs;
pthread_mutex_t             faceAgent_cs;
FaceAgent::FaceAgent(unsigned short localPort, char*photoBasePath, char*targetsFolderPath)
{
    this->photoBasePath = new char[strlen(photoBasePath) + 1];
    this->targetsFolderPath = new char[strlen(targetsFolderPath) + 1];
    strcpy(this->photoBasePath, photoBasePath);
    strcpy(this->photoBasePath, photoBasePath);

    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&faceAgent_trace_cs, &mAttr);
    pthread_mutex_init(&faceAgent_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadServerListener = new CommonThread;
    localSock = INVALID_SOCKET;

    this->localPortNumber = localPort;
    FACE_AGENT_TRACE(FaceAgent, "new FaceAgent");
}

FaceAgent::~FaceAgent()
{
    delete []photoBasePath;
    delete []targetsFolderPath;

    shutdown(localSock, 2);
    threadServerListener->stopThread();
    delete threadServerListener;

    pthread_mutex_destroy(&faceAgent_cs);
    pthread_mutex_destroy(&faceAgent_trace_cs);
    FACE_AGENT_TRACE(FaceAgent, "~FaceAgent");
}

bool FaceAgent::StartFaceAgent()
{
    FACE_AGENT_TRACE(StartFaceAgent, "Calling InitFaceCommonLib");
    if(!InitFaceCommonLib("FaceAgent.log"))
    {
        FACE_AGENT_TRACE(StartFaceAgent, "InitFaceCommonLib failed");
        return false;
    }
    FACE_AGENT_TRACE(StartFaceAgent, "InitFaceCommonLib succeed");
    FACE_AGENT_TRACE(StartFaceAgent, "Calling StartNetworkServer");
    NetResult res;
    if(NET_SUCCESS != (res = StartNetworkServer()))
    {
        FACE_AGENT_TRACE(StartFaceAgent, "StartNetworkServer failed on error %x", res);
        return false;
    }
    FACE_AGENT_TRACE(StartFaceAgent, "StartNetworkServer succeed");

    return true;
}

NetResult FaceAgent::StartNetworkServer()
{
    sockaddr_in             server;
    memset(&server, 0, sizeof(server));

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(localPortNumber);

    if ((localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        FACE_AGENT_TRACE(StartNetworkServer, "socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    //[TBD] think about SO_KEEPALIVE option

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "setsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);

    if(0 != getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "getsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        FACE_AGENT_TRACE(StartNetworkServer, "setsockopt failed to enabled SO_REUSEADDR option");
        //return NET_SOCKET_ERROR;
    }

    if(0 != bind(localSock, (struct sockaddr*)&server, sizeof(server)))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "bind failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    if (0 != listen(localSock, SOMAXCONN))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "listen failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_UNSPECIFIED_ERROR;
    }

    FaceAgent *pThis = this;

    FACE_AGENT_TRACE(StartNetworkServer, "Starting ServerListener", strerror(errno));

    if(!threadServerListener->startThread((void*(*)(void*))FaceAgent::ServerListener, &pThis, sizeof(FaceAgent*)))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "Failed to start ServerListener thread. See CommonThread logs for information");
        return NET_COMMON_THREAD_ERROR;
    }

    FACE_AGENT_TRACE(StartNetworkServer, "Succeed, socket = %d, port = %d", localSock, localPortNumber);

    return NET_SUCCESS;
}

bool FaceAgent::StopFaceAgent()
{
    bool success = true;

    NetResult res;

    FACE_AGENT_TRACE(StopFaceAgent, "Calling StopNetworkServer");
    if(NET_SUCCESS != (res = StopNetworkServer()))
    {
        FACE_AGENT_TRACE(StopFaceAgent, "StopNetworkServer failed on result %x. Continue...", res);
        success = false;
    }
    FACE_AGENT_TRACE(StopFaceAgent, "StopNetworkServer succeed");
    FACE_AGENT_TRACE(StopFaceAgent, "Calling DeinitFaceCommonLib");
    if(!DeinitFaceCommonLib())
    {
        FACE_AGENT_TRACE(StopFaceAgent, "DeinitFaceCommonLib failed");
        success = false;
    }
    FACE_AGENT_TRACE(StopFaceAgent, "DeinitFaceCommonLib succeed");

    return success;
}

void FaceAgent::ServerListener(void* param)
{
    FaceAgent                  *pThis                       = NULL;
    SOCKET                      accepted_socket             = INVALID_SOCKET;
    int                         dataLength                  = 0;
    char                        data[MAX_SOCKET_BUFF_SIZE]  = {0};
    std::vector<std::string>    mandatoryKeys;

    memcpy(&pThis, param, sizeof(FaceAgent*));

    mandatoryKeys.push_back("cmd");
    mandatoryKeys.push_back("req_id");
    mandatoryKeys.push_back("reply_sock");
    mandatoryKeys.push_back("result");

    for(;;)
    {
        FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
        if ((accepted_socket = accept(pThis->localSock, NULL, NULL)) == SOCKET_ERROR)
        {
            if(pThis->threadServerListener->isStopThreadReceived())
            {
                FACE_AGENT_TRACE(ServerListener, "terminate thread sema received");
                break;
            }
            continue;
        }

        FACE_AGENT_TRACE(AcceptConnection, "Incoming connection accepted. Accepted socket = %u", accepted_socket);

        int flag = 1;   //TRUE
        if(0 != setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)))
        {
            FACE_AGENT_TRACE(AcceptConnection, "setsockopt (TCP_NODELAY) failed on error %s", strerror(errno));
            continue;
        }
        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
        {
            FACE_AGENT_TRACE(AcceptConnection, "setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            continue;
        }

        FACE_AGENT_TRACE(ServerListener, "Socket listener cycle started. All incoming connections are ingored");
        for(;;)
        {
            if(pThis->threadServerListener->isStopThreadReceived())
            {
                FACE_AGENT_TRACE(ServerListener, "terminate thread sema received");
                break;
            }
            if( -1 == (dataLength = recv(accepted_socket, data, sizeof(data), MSG_DONTWAIT)))
            {
                if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    // Valid errors - continue
                    continue;
                }
                // unspecified error occured
                FACE_AGENT_TRACE(ServerListener, "recv failed on error %s", strerror(errno));
                FACE_AGENT_TRACE(ServerListener, "SocketListener shutdown");
                shutdown(accepted_socket, 2);
                break;
            }

            if(dataLength == 0)
            {
                // TCP keep alive
                continue;
            }

            FACE_AGENT_TRACE(ServerListener, "Received %d bytes from the server(%u)", dataLength, accepted_socket);

            // Response from agent received
            json::Object requestJson;
            try
            {
                requestJson = ((json::Value)json::Deserialize((std::string)data)).ToObject();
            }
            catch (...)
            {
                FACE_AGENT_TRACE(ServerListener, "Failed to parse incoming JSON: %s", data);
                continue;
            }

            if (!(requestJson.HasKeys(mandatoryKeys)))
            {
                FACE_AGENT_TRACE(ServerListener, "Invalid input JSON: no cmd field (%s)", data);
                continue;
            }

            requestJson["result"] = "success";

            std::string outJson = json::Serialize(requestJson);

            FACE_AGENT_TRACE(ServerListener, "Simply echo with success result", dataLength, accepted_socket);

            if(NET_SUCCESS != pThis->SendData(accepted_socket, outJson.c_str(), outJson.size()))
            {
                FACE_AGENT_TRACE(ServerListener, "Failed to send response to server (%u). Reponse = %s", accepted_socket, outJson.c_str());
                continue;
            }
        }
    }
    shutdown(pThis->localSock, 2);
    FACE_AGENT_TRACE(ServerListener, "AcceptConnection finished");
    return;
}

NetResult FaceAgent::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if (-1 == (sendlen = send(sock, pBuffer, uBufferSize, 0)))
        {
            FACE_AGENT_TRACE(SendData, "Failed to send outgoing bytes to the remote peer %u with error %s", sock, strerror(errno));
            return NET_SOCKET_ERROR;
        }

        usleep(50000);      // sleep for 1 system monitor tact
        FACE_AGENT_TRACE(SendData, "%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        FACE_AGENT_TRACE(SendData, "Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}

NetResult FaceAgent::StopNetworkServer()
{
    NetResult res = NET_SUCCESS;

    FACE_AGENT_TRACE(StopNetworkServer, "Calling socket shutdown. localSock = %i", localSock);
    if(-1 == shutdown(localSock, 2))      // 2 = Both reception and transmission
    {
        FACE_AGENT_TRACE(StopNetworkServer, "Failed to shutdown local socket with error = %s", strerror(errno));
        res = NET_SOCKET_ERROR;
    }
    FACE_AGENT_TRACE(StopNetworkServer, "socket shutdown succeed");

    FACE_AGENT_TRACE(StopNetworkServer, "Calling threadServerListener->stopThread");
    if(!threadServerListener->stopThread())
    {
        FACE_AGENT_TRACE(StopNetworkServer, "threadServerListener->stopThread failed");
        res = NET_COMMON_THREAD_ERROR;
    }
    FACE_AGENT_TRACE(StopNetworkServer, "threadServerListener->stopThread succeed");

    return res;
}

/*
#include "LibInclude.h"
#include <ctime>

FaceCascades *cascades;
map < string, Ptr<FaceRecognizer> > models;

void getFacesFromPhoto(void *pContext)
{
    double startTime = clock();
    ContextForGetFaces *psContext = (ContextForGetFaces*)pContext;

    string photoName = ((string)ID_PATH).append(psContext->userId).append("/photos/").append(psContext->photoName).append(".jpg");

    IplImage *img = cvLoadImage(photoName.c_str());

    if (img == NULL)
    {
        FilePrintMessage(_FAIL("Failed to load image %s. Continue..."), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"failed to load photo\" }\n\0", strlen("{ \"error\":\"failed to load photo\" }\n\0"));

        return;
    }

    ViolaJonesDetection detector(cascades);

    try{
        if (!detector.allFacesDetection(img, psContext->sock))
        {
            FilePrintMessage(_FAIL("All Faces Detection FAILED"), photoName.c_str());
            net.SendData(psContext->sock, "{ \"error\":\"All Faces Detection FAILED\" }\n\0", strlen("{ \"error\":\"All Faces Detection FAILED\" }\n\0"));

            return;
        }
    }
    catch (...)
    {
        FilePrintMessage(_FAIL("All Faces Detection FAILED"), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"All Faces Detection FAILED\" }\n\0", strlen("{ \"error\":\"All Faces Detection FAILED\" }\n\0"));

        return;
    }

    net.SendData(psContext->sock, "{ \"success\":\"get faces succeed\" }\n\0", strlen("{ \"success\":\"get faces succeed\" }\n\0"));

    FilePrintMessage(_SUCC("Get faces finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);

    return;
}

void saveFaceFromPhoto(void *pContext)
{
    double startTime = clock();
    ContextForSaveFaces *psContext = (ContextForSaveFaces*)pContext;

    string photoName = ((string)ID_PATH).append(psContext->userId).append("/photos/").append(psContext->photoName).append(".jpg");

    IplImage *img = cvLoadImage(photoName.c_str());

    if (img == NULL)
    {
        FilePrintMessage(_FAIL("Failed to load image %s. Continue..."), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"failed to load photo\" }\n\0", strlen("{ \"error\":\"failed to load photo\" }\n\0"));

        return;
    }

    ViolaJonesDetection detector(cascades);

    try
    {
        if (!detector.cutTheFace(img, ((string)ID_PATH).append(psContext->userId).append("/faces/").append(psContext->photoName).append(".jpg").c_str(), psContext->faceNumber))
        {
            FilePrintMessage(_FAIL("cut face FAILED"), photoName.c_str());
            net.SendData(psContext->sock, "{ \"error\":\"cut face FAILED\" }\n\0", strlen("{ \"error\":\"cut face FAILED\" }\n\0"));

            return;
        }
    }
    catch (...)
    {
        FilePrintMessage(_FAIL("cut face FAILED"), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"cut face FAILED\" }\n\0", strlen("{ \"error\":\"cut face FAILED\" }\n\0"));

        return;
    }

    net.SendData(psContext->sock, "{ \"success\":\"cut face succeed\" }\n\0", strlen("{ \"success\":\"cut face succeed\" }\n\0"));

    FilePrintMessage(_SUCC("Cut face finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);

    return;
}

void recognizeFromModel(void *pContext)
{
    double startTime = clock();
    ContextForRecognize *psContext = (ContextForRecognize*)pContext;
    //CvMemStorage* storage = NULL;
    IplImage *img = NULL;
    ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection(cascades);
    map < string, Ptr<FaceRecognizer> > currentModels;

    for (unsigned long i = 0; i < psContext->arrFrinedsList.size(); i++)
    {
        map < string, Ptr<FaceRecognizer> >::iterator it;
        it = models.find(psContext->arrFrinedsList[i].ToString());
        if (it != models.end())
        {
            currentModels[psContext->arrFrinedsList[i].ToString()] = models[psContext->arrFrinedsList[i].ToString()];
        }
        else
        {
            FilePrintMessage(_WARN("No model found for user %s"), psContext->arrFrinedsList[i].ToString().c_str());
        }
    }

    if (currentModels.empty())
    {
        FilePrintMessage(_FAIL("No models loaded."));
        net.SendData(psContext->sock, "{ \"error\":\"training was not called\" }\n\0", strlen("{ \"error\":\"training was not called\" }\n\0"));
        delete violaJonesDetection;
        return;
    }

    try
    {
        img = cvLoadImage(((string)TARGET_PATH).append(psContext->targetImg).append(".jpg").c_str());
    }
    catch (...)
    {
        FilePrintMessage(_FAIL("Failed to load image %s"), ((string)(TARGET_PATH)).append(psContext->targetImg).append(".jpg").c_str());
        net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
        delete violaJonesDetection;
        return;
    }

    if (!img)
    {
        FilePrintMessage(_FAIL("Failed to load image %s"), (((string)(TARGET_PATH)).append(psContext->targetImg)).c_str());
        net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
        delete violaJonesDetection;
        return;
    }

    //storage = cvCreateMemStorage();
    try
    {
        if (!violaJonesDetection->faceDetect(img, currentModels, psContext->sock))
        {
            FilePrintMessage(_FAIL("Some error occured during recognze call"));
            net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
            delete violaJonesDetection;
            return;
        }
    }
    catch (...)
    {
        FilePrintMessage(_FAIL("Some error occured during recognze call"));
        net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
        delete violaJonesDetection;
        return;
    }

#ifdef SHOW_IMAGE
    while (1){
        if (cvWaitKey(0) == 27)
            break;
    }
#endif

    net.SendData(psContext->sock, "{ \"success\":\"recognize faces succeed\" }\n\0", strlen("{ \"success\":\"recognize faces succeed\" }\n\0"));

    FilePrintMessage(_SUCC("Recognize finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
    cvReleaseImage(&img);
    //cvClearMemStorage(storage);
    cvDestroyAllWindows();
    delete violaJonesDetection;
    return;
}

void generateAndTrainBase(void *pContext)
{
    pContext = pContext;
    double startTime = clock();

    ContextForTrain *psContext = (ContextForTrain*)pContext;

    __int64_t uSuccCounter = 0;

    for (__int64_t i = 0; i < psContext->arrIds.size(); i++)
    {
        __int64_t uNumOfThreads = 0;

        vector<string> photos = vector<string>();
        getFilesFromDir(((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("/photos/").c_str(), photos);

        vector<CommonThread *> threads;

        for (unsigned int j = 0; j < photos.size(); j++)
        {
            string photoName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("/photos/").append(photos[j]);
            IplImage *img = cvLoadImage(photoName.c_str());
            if (img == NULL)
            {
                FilePrintMessage(_WARN("Failed to load image %s. Continue..."), photoName.c_str());
                continue;
            }

            cutFaceThreadParams * param = new cutFaceThreadParams(img,
                (((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("/faces/").append(photos[j])).c_str(),
                &cascades[uNumOfThreads]);
            CommonThread *threadCutFace = new CommonThread;
            threadCutFace->startThread((void*(*)(void*))param->pThis->cutFaceThread, param, sizeof(cutFaceThreadParams));
            threads.push_back( threadCutFace);

            if (++uNumOfThreads == MAX_THREADS_AND_CASCADES_NUM)
            {
                for(vector<CommonThread*>::iterator it = threads.begin(); it != threads.end(); ++it)
                {
                    ((CommonThread*)*it)->waitForFinish(CUT_TIMEOUT);
                    ((CommonThread*)*it)->stopThread();
                    delete *it;
                }
                threads.clear();

                uNumOfThreads = 0;
            }
        }

        for(vector<CommonThread*>::iterator it = threads.begin(); it != threads.end(); ++it)
        {
            ((CommonThread*)*it)->waitForFinish(CUT_TIMEOUT);
            ((CommonThread*)*it)->stopThread();
            delete *it;
        }
        threads.clear();

        EigenDetector *eigenDetector = new EigenDetector();

        //train FaceRecognizer
        try
        {
            if (!eigenDetector->train((((string)ID_PATH).append(psContext->arrIds[i].ToString())).c_str()))
            {
                FilePrintMessage(_FAIL("Some error has occured during Learn call."));
                delete eigenDetector;
                continue;
            }
        }
        catch (...)
        {
            FilePrintMessage(_FAIL("Some error has occured during Learn call."));
            delete eigenDetector;
            continue;
        }

        pthread_mutex_lock(&faceDetectionCS);
        Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
        try
        {
            string fileName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("/eigenface.yml");
            if (access(fileName.c_str(), 0) != -1)
            {
                model->load(fileName.c_str());
                FilePrintMessage(_SUCC("Model base for user %s successfully loaded. Continue..."), psContext->arrIds[i].ToString().c_str());
            }
            else
            {
                FilePrintMessage(_WARN("Failed to load model base for user %s. Continue..."), psContext->arrIds[i].ToString().c_str());
                pthread_mutex_unlock(&faceDetectionCS);
                continue;
            }

        }
        catch (...)
        {
            FilePrintMessage(_WARN("Failed to load model base for user %s. Continue..."), psContext->arrIds[i].ToString().c_str());
            pthread_mutex_unlock(&faceDetectionCS);
            continue;
        }

        models[psContext->arrIds[i].ToString()] = model;
        pthread_mutex_unlock(&faceDetectionCS);

        delete eigenDetector;
        uSuccCounter++;
    }

    cvDestroyAllWindows();
    FilePrintMessage(_SUCC("Train finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
    if (uSuccCounter == 0)
    {
        net.SendData(psContext->sock, "{ \"fail\":\"learning failed\" }\n\0", strlen("{ \"fail\":\"learning failed\" }\n\0"));
        return;
    }

    net.SendData(psContext->sock, "{ \"success\":\"train succeed\" }\n\0", strlen("{ \"success\":\"train succeed\" }\n\0"));
    return;
}





















*/
