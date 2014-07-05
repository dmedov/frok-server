#include "stdafx.h"
#include "LibInclude.h"
#include <ctime>

FaceCascades cascades[MAX_THREADS_AND_CASCADES_NUM];
map <string, Ptr<FaceRecognizer>> models;

void getFacesFromPhoto(void *pContext)
{
    double startTime = clock();
    ContextForGetFaces *psContext = (ContextForGetFaces*)pContext;

    string photoName = ((string)ID_PATH).append(psContext->userId).append("\\photos\\").append(psContext->photoName).append(".jpg");

    IplImage *img = cvLoadImage(photoName.c_str());

    if (img == NULL)
    {
        FilePrintMessage(NULL, _FAIL("Failed to load image %s. Continue..."), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"failed to load photo\" }\n\0", strlen("{ \"error\":\"failed to load photo\" }\n\0"));
        delete psContext;
        return;
    }

    ViolaJonesDetection detector(cascades);

    try{
        if (!detector.allFacesDetection(img, psContext->sock))
        {
            FilePrintMessage(NULL, _FAIL("All Faces Detection FAILED"), photoName.c_str());
            net.SendData(psContext->sock, "{ \"error\":\"All Faces Detection FAILED\" }\n\0", strlen("{ \"error\":\"All Faces Detection FAILED\" }\n\0"));
            delete psContext;
            return;
        }
    }
    catch (...)
    {
        FilePrintMessage(NULL, _FAIL("All Faces Detection FAILED"), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"All Faces Detection FAILED\" }\n\0", strlen("{ \"error\":\"All Faces Detection FAILED\" }\n\0"));
        delete psContext;
        return;
    }

    net.SendData(psContext->sock, "{ \"success\":\"get faces succeed\" }\n\0", strlen("{ \"success\":\"get faces succeed\" }\n\0"));

    FilePrintMessage(NULL, _SUCC("Get faces finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
    delete psContext;
    return;
}

void saveFaceFromPhoto(void *pContext)
{
    double startTime = clock();
    ContextForSaveFaces *psContext = (ContextForSaveFaces*)pContext;

    string photoName = ((string)ID_PATH).append(psContext->userId).append("\\photos\\").append(psContext->photoName).append(".jpg");

    IplImage *img = cvLoadImage(photoName.c_str());

    if (img == NULL)
    {
        FilePrintMessage(NULL, _FAIL("Failed to load image %s. Continue..."), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"failed to load photo\" }\n\0", strlen("{ \"error\":\"failed to load photo\" }\n\0"));
        delete psContext;
        return;
    }

    ViolaJonesDetection detector(cascades);

    try
    {
        if (!detector.cutTheFace(img, ((string)ID_PATH).append(psContext->userId).append("\\faces\\").append(psContext->photoName).append(".jpg").c_str(), psContext->faceNumber))
        {
            FilePrintMessage(NULL, _FAIL("cut face FAILED"), photoName.c_str());
            net.SendData(psContext->sock, "{ \"error\":\"cut face FAILED\" }\n\0", strlen("{ \"error\":\"cut face FAILED\" }\n\0"));
            delete psContext;
            return;
        }
    }
    catch (...)
    {
        FilePrintMessage(NULL, _FAIL("cut face FAILED"), photoName.c_str());
        net.SendData(psContext->sock, "{ \"error\":\"cut face FAILED\" }\n\0", strlen("{ \"error\":\"cut face FAILED\" }\n\0"));
        delete psContext;
        return;
    }

    net.SendData(psContext->sock, "{ \"success\":\"cut face succeed\" }\n\0", strlen("{ \"success\":\"cut face succeed\" }\n\0"));

    FilePrintMessage(NULL, _SUCC("Cut face finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
    delete psContext;
    return;
}

void recognizeFromModel(void *pContext)
{
    double startTime = clock();
    ContextForRecognize *psContext = (ContextForRecognize*)pContext;
    //CvMemStorage* storage = NULL;
    IplImage *img = NULL;
    ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection(cascades);
    map <string, Ptr<FaceRecognizer>> currentModels;

    for (unsigned long i = 0; i < psContext->arrFrinedsList.size(); i++)
    {
        map <string, Ptr<FaceRecognizer>>::iterator it;
        it = models.find(psContext->arrFrinedsList[i].ToString());
        if (it != models.end())
        {
            currentModels[psContext->arrFrinedsList[i].ToString()] = models[psContext->arrFrinedsList[i].ToString()];
        }
        else
        {
            FilePrintMessage(NULL, _WARN("No model found for user %s"), psContext->arrFrinedsList[i].ToString().c_str());
        }
    }


    if (currentModels.empty())
    {
        FilePrintMessage(NULL, _FAIL("No models loaded."));
        net.SendData(psContext->sock, "{ \"error\":\"training was not called\" }\n\0", strlen("{ \"error\":\"training was not called\" }\n\0"));
        delete violaJonesDetection;
        delete psContext;
        return;
    }

    try
    {
        img = cvLoadImage(((string)(TARGET_PATH)).append(psContext->targetImg.append(".jpg")).c_str());
    }
    catch (...)
    {
        FilePrintMessage(NULL, _FAIL("Failed to load image %s"), (((string)(TARGET_PATH)).append(psContext->targetImg)).c_str());
        net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
        delete violaJonesDetection;
        delete psContext;
        return;
    }


    if (!img)
    {
        FilePrintMessage(NULL, _FAIL("Failed to load image %s"), (((string)(TARGET_PATH)).append(psContext->targetImg)).c_str());
        net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
        delete violaJonesDetection;
        delete psContext;
        return;
    }

    //storage = cvCreateMemStorage();                    // �������� ��������� ������
    try
    {
        if (!violaJonesDetection->faceDetect(img, currentModels, psContext->sock))
        {
            FilePrintMessage(NULL, _FAIL("Some error occured during recognze call"));
            net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
            delete violaJonesDetection;
            delete psContext;
            return;
        }
    }
    catch (...)
    {
        FilePrintMessage(NULL, _FAIL("Some error occured during recognze call"));
        net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
        delete violaJonesDetection;
        delete psContext;
        return;
    }

#ifdef SHOW_IMAGE
    while (1){
        if (cvWaitKey(0) == 27)
            break;
    }
#endif

    net.SendData(psContext->sock, "{ \"success\":\"recognize faces succeed\" }\n\0", strlen("{ \"success\":\"recognize faces succeed\" }\n\0"));

    FilePrintMessage(NULL, _SUCC("Recognize finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
    cvReleaseImage(&img);
    //cvClearMemStorage(storage);
    cvDestroyAllWindows();
    delete violaJonesDetection;
    delete psContext;
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
        getFilesFromDir(((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\photos\\").c_str(), photos);

        vector<CommonThread *> threads;

        for (unsigned int j = 0; j < photos.size(); j++)
        {
            string photoName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\photos\\").append(photos[j]);
            IplImage *img = cvLoadImage(photoName.c_str());
            if (img == NULL)
            {
                FilePrintMessage(NULL, _WARN("Failed to load image %s. Continue..."), photoName.c_str());
                continue;
            }

            cutFaceThreadParams * param = new cutFaceThreadParams(img,
                (((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\faces\\").append(photos[j])).c_str(),
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
                FilePrintMessage(NULL, _FAIL("Some error has occured during Learn call."));
                delete eigenDetector;
                continue;
            }
        }
        catch (...)
        {
            FilePrintMessage(NULL, _FAIL("Some error has occured during Learn call."));
            delete eigenDetector;
            continue;
        }

        pthread_mutex_lock(&faceDetectionCS);
        Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
        try
        {
            string fileName = ((string)(ID_PATH)).append(psContext->arrIds[i].ToString()).append("//eigenface.yml");
            if (access(fileName.c_str(), 0) != -1)
            {
                model->load(fileName.c_str());
                FilePrintMessage(NULL, _SUCC("Model base for user %s successfully loaded. Continue..."), psContext->arrIds[i].ToString().c_str());
            }
            else
            {
                FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), psContext->arrIds[i].ToString().c_str());
                pthread_mutex_unlock(&faceDetectionCS);
                continue;
            }

        }
        catch (...)
        {
            FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), psContext->arrIds[i].ToString().c_str());
            pthread_mutex_unlock(&faceDetectionCS);
            continue;
        }

        models[psContext->arrIds[i].ToString()] = model;
        pthread_mutex_unlock(&faceDetectionCS);

        delete eigenDetector;
        uSuccCounter++;
    }

    cvDestroyAllWindows();
    FilePrintMessage(NULL, _SUCC("Train finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
    if (uSuccCounter == 0)
    {
        net.SendData(psContext->sock, "{ \"fail\":\"learning failed\" }\n\0", strlen("{ \"fail\":\"learning failed\" }\n\0"));
        return;
    }

    net.SendData(psContext->sock, "{ \"success\":\"train succeed\" }\n\0", strlen("{ \"success\":\"train succeed\" }\n\0"));
    return;
}
