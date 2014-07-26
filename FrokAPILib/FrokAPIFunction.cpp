#include "FrokAPIFunction.h"

#define MODULE_NAME     "FROK_API"

FrokResult GetFacesFromPhoto(void *pContext)
{
    UNREFERENCED_PARAMETER(pContext);
    /*double startTime = clock();
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
*/
    return FROK_RESULT_SUCCESS;
}

FrokResult AddFaceFromPhoto(void *pContext)
{
    UNREFERENCED_PARAMETER(pContext);
/*    double startTime = clock();
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
*/
    return FROK_RESULT_SUCCESS;
}

FrokResult Recognize(void *pContext)
{
    UNREFERENCED_PARAMETER(pContext);
/*    double startTime = clock();
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
    delete violaJonesDetection;*/
    return FROK_RESULT_SUCCESS;
}

FrokResult TrainUserModel(std::vector<std::string> ids, const char *userBasePath, FrokFaceDetector &detector, FrokFaceRecognizer &recognizer)
{
    timespec startTime;
    timespec endTime;
    bool isSuccess = true;
    TRACE_T("Training started");
    if(ids.empty())
    {
        TRACE_F_T("Invalid parameter: ids vector is empty");
        return FROK_RESULT_INVALID_PARAMETER;
    }

    FrokResult res;

    for(std::vector<std::string>::const_iterator it = ids.begin(); it != ids.end(); ++it)
    {
        memset(&startTime, 0, sizeof(startTime));
        memset(&endTime, 0, sizeof(endTime));

        printf("Starting train for user %s\n", ((std::string)*it).c_str());
        clock_gettime(CLOCK_REALTIME, &startTime);

        std::string currentUserFolder = userBasePath;
        currentUserFolder.append(*it).append("/photos/");

        std::vector<std::string> userPhotos;

        if(-1 == getFilesFromDir(currentUserFolder.c_str(), userPhotos))
        {
            TRACE_F_T("Failed to get photos from directory %s", currentUserFolder.c_str());
            continue;
        }

        TRACE_T("Found %d photos for user %s", userPhotos.size(), ((std::string)*it).c_str());

        if(!userPhotos.empty())
        {
            unsigned successCounter = 0;

            for(std::vector<std::string>::iterator iterImage = userPhotos.begin(); iterImage != userPhotos.end(); ++iterImage)
            {
                std::string imageFullPath = currentUserFolder;
                imageFullPath.append((std::string)*iterImage);
                if(FROK_RESULT_SUCCESS != (res = detector.SetTargetImage(imageFullPath.c_str())))
                {
                    TRACE_F_T("Failed to SetTargetImage on result %s", FrokResultToString(res));
                    continue;
                }

                std::vector<cv::Rect> faces;

                if(FROK_RESULT_SUCCESS != (res = detector.GetFacesFromPhoto(faces)))
                {
                    TRACE_F_T("Failed to GetFacesFromPhoto on result %s", FrokResultToString(res));
                    continue;
                }

                std::vector<cv::Mat> faceImages;

                if(FROK_RESULT_SUCCESS != (res = detector.GetNormalizedFaceImages(faces, faceImages)))
                {
                    TRACE_F_T("Failed to GetNormalizedFaceImages on result %s", FrokResultToString(res));
                    continue;
                }

                FaceUserModel *model;
                try
                {
                    model = new FaceUserModel(*it, RECOGNIZER_EIGENFACES);
                }
                catch(FrokResult error)
                {
                    TRACE_F_T("Failed to create FaceUserModel on error %s", FrokResultToString(error));
                    continue;
                }
                catch(...)
                {
                    TRACE_F_T("Unknown error on FaceUserModel creation.");
                    continue;
                }

                if(FROK_RESULT_SUCCESS != (res = model->GenerateUserModel(faceImages)))
                {
                    TRACE_F_T("Failed to GenerateUserModel on result %s", FrokResultToString(res));
                    continue;
                }

                if(FROK_RESULT_SUCCESS != (res = model->SaveUserModel(currentUserFolder.c_str())))
                {
                    TRACE_F_T("Failed to SaveUserModel on result %s", FrokResultToString(res));
                    continue;
                }

                if(FROK_RESULT_SUCCESS != (res = recognizer.AddFrokUserModel(((std::string)*it), *model)))
                {
                    TRACE_F_T("Failed to AddFrokUserModel on result %s", FrokResultToString(res));
                    continue;
                }

                successCounter++;
            }

            if(successCounter == 0)
            {
                isSuccess = false;
            }
        }

        clock_gettime(CLOCK_REALTIME, &endTime);

        printf("Training for user %s finished\n", ((std::string)*it).c_str());
        print_time(startTime, endTime);
    }

    if(isSuccess == false)
    {
        TRACE_F_T("Training failed for one or more user - return failure");
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    TRACE_T("Training finished");

    return FROK_RESULT_SUCCESS;
}

