#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>

#include "FaceAgent.h"

pthread_mutex_t             faceServer_cs;
FaceAgent::FaceAgent(unsigned short localPort, char*photoBasePath, char*targetsFolderPath)
    : Network(DefaultCallback, localPort)
{
    this->photoBasePath = new char[strlen(photoBasePath) + 1];
    this->targetsFolderPath = new char[strlen(targetsFolderPath) + 1];
    strcpy(this->photoBasePath, photoBasePath);
    strcpy(this->photoBasePath, photoBasePath);
}

FaceAgent::~FaceAgent()
{
    delete []photoBasePath;
    delete []targetsFolderPath;
}

bool FaceAgent::StartFaceAgent()
{
    InitFaceCommonLib();

    if(NET_SUCCESS != StartNetworkServer())
    {
        return false;
    }

    return true;
}

bool FaceAgent::StopFaceAgent()
{
    bool success = true;

    if(NET_SUCCESS != StopNetworkServer())
    {
        success = false;
    }

    DeinitFaceCommonLib();

    return success;
}

void FaceAgent::CallbackListener(void *pContext)
{
    SocketListenerData     *psParam = (SocketListenerData*)pContext;
    FaceAgent     *pThis = (FaceAgent*)psParam->pThis;

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
            FilePrintMessage(_FAIL("Invalid input JSON: no cmd field (%s)"), data);
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
