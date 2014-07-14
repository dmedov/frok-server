#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>

#include "FaceAgent.h"

std::list <FaceRequest *> requests;
sem_t                       newRequestSema;
pthread_mutex_t             faceServer_cs;
FaceAgent::FaceAgent(unsigned short localPort, char*photoBasePath, char*targetsFolderPath)
    : Network(DefaultCallback, localPort)
{
    this->photoBasePath = new char[strlen(photoBasePath) + 1];
    this->targetsFolderPath = new char[strlen(targetsFolderPath) + 1];
    strcpy(this->photoBasePath, photoBasePath);
    strcpy(this->photoBasePath, photoBasePath);

    this->localPort = localPort;

    sem_init(&newRequestSema, 0, 0);

    network = new Network(FaceServer::NetworkCallback, this->localPort);
    threadCallbackListener = new CommonThread();
}

FaceAgent::~FaceAgent()
{
    delete []photoBasePath;
    delete []targetsFolderPath;
    delete network;
    delete threadCallbackListener;
}

bool FaceAgent::StartAgent()
{
    InitFaceCommonLib();

    if(NET_SUCCESS != network->StartNetworkServer())
    {
        return false;
    }

    return true;
}

bool FaceAgent::StopFaceAgent()
{
    bool success = true;

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

void FaceAgent::NetworkCallback(unsigned evt, SOCKET sock, unsigned length, void *param)
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
        /*json::Object requestJson;
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
        if (objInputJson["cmd"].ToString() == NET_CMD_RECOGNIZE)
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
        else if (objInputJson["cmd"].ToString() == NET_CMD_GET_FACES)
        {
            if (!objInputJson.HasKey("user_id"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("photo_id"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                return;
            }

            ContextForGetFaces *psContext = new ContextForGetFaces;
            psContext->userId = objInputJson["user_id"].ToString();
            psContext->photoName = objInputJson["photo_id"].ToString();
            psContext->sock = sock;

            FilePrintMessage(_SUCC("Getting faces started..."));
            CommonThread *threadGetFaces = new CommonThread;
            threadGetFaces->startThread((void*(*)(void*))getFacesFromPhoto, psContext, sizeof(ContextForGetFaces));
            // Notice that psContext should be deleted in recognizeFromModel function!
        }
        else if (objInputJson["cmd"].ToString() == NET_CMD_SAVE_FACE)
        {
            if (!objInputJson.HasKey("user_id"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("photo_id"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                return;
            }

            if (!objInputJson.HasKey("face_number"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no face_number field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no face_points field\" }\n\0", strlen("{ \"error\":\"no face_number field\" }\n\0"));
                return;
            }

            ContextForSaveFaces *psContext = new ContextForSaveFaces;
            psContext->userId = objInputJson["user_id"].ToString();
            psContext->photoName = objInputJson["photo_id"].ToString();
            psContext->faceNumber = atoi(objInputJson["face_number"].ToString().c_str());
            psContext->sock = sock;

            FilePrintMessage(_SUCC("Cut face started..."));
            CommonThread *threadSaveFaces = new CommonThread;
            threadSaveFaces->startThread((void*(*)(void*))saveFaceFromPhoto, psContext, sizeof(ContextForSaveFaces));
            // Notice that psContext should be deleted in recognizeFromModel function!
        }
        else if (objInputJson["cmd"].ToString() == NET_CMD_TRAIN)
        {
            if (!objInputJson.HasKey("ids"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no ids field (%s)"), (char*)param);
                network->SendData(sock, "{ \"error\":\"no ids field\" }\n\0", strlen("{ \"error\":\"no ids field\" }\n\0"));
                return;
            }

            ContextForTrain *psContext = new ContextForTrain;
            psContext->arrIds = objInputJson["ids"].ToArray();
            psContext->sock = sock;

            FilePrintMessage(_SUCC("Training started..."));
            CommonThread *threadTrain = new CommonThread;
            threadTrain->startThread((void*(*)(void*))generateAndTrainBase, (void*)psContext, sizeof(ContextForTrain));

            // Notice that psContext should be deleted in recognizeFromModel function!
        }
        else
        {
            FilePrintMessage(_FAIL("Invalid input JSON: invalid cmd received (%s)"), (char*)param);
            network->SendData(sock, "{ \"error\":\"invalid cmd\" }\n\0", strlen("{ \"error\":\"invalid cmd\" }\n\0"));
            return;
        }
        break;*/
    }
}

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





















