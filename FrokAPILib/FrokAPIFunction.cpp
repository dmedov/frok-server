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

FrokResult Recognize(std::vector< std::map<std::string, double> > &similarities, std::vector<std::string> ids, const char *userBasePath, std::string photoName, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
{
    timespec startTime;
    timespec endTime;

    TRACE_T("Recognizing started");
    similarities.clear();
    if(ids.empty())
    {
        TRACE_F_T("Invalid parameter: ids vector is empty");
        return FROK_RESULT_INVALID_PARAMETER;
    }

    if((userBasePath == NULL) || (targetPhotosPath == NULL))
    {
        TRACE_F_T("Invalid parameter: userBasePath = %p, targetPhotoPath = %p", userBasePath, targetPhotosPath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    FrokResult res;

    memset(&startTime, 0, sizeof(startTime));
    memset(&endTime, 0, sizeof(endTime));

    printf("Starting recognition\n");
    clock_gettime(CLOCK_REALTIME, &startTime);

    TRACE_T("Loading models for requested ids");

    if(FROK_RESULT_SUCCESS != (res = recognizer->SetUserIdsVector(ids)))
    {
        TRACE_W_T("Failed to SetUserIdsVector on error %s", FrokResultToString(res));
        return res;
    }

    TRACE_S_T("Loading models succeed");

    std::string targetImageFullPath = targetPhotosPath;
    targetImageFullPath.append(photoName);

    TRACE_T("Detecting faces on target photo %s", targetImageFullPath.c_str());

    if(FROK_RESULT_SUCCESS != (res = detector->SetTargetImage(targetImageFullPath.c_str())))
    {
        TRACE_F_T("Failed to SetTargetImage on result %s", FrokResultToString(res));
        return res;
    }

    std::vector<cv::Rect> faces;

    if(FROK_RESULT_SUCCESS != (res = detector->GetFacesFromPhoto(faces)))
    {
        TRACE_F_T("Failed to GetFacesFromPhoto on result %s", FrokResultToString(res));
        return res;
    }

    std::vector<cv::Mat> faceImages;

    if(FROK_RESULT_SUCCESS != (res = detector->GetNormalizedFaceImages(faces, faceImages)))
    {
        TRACE_F_T("Failed to GetNormalizedFaceImages on result %s", FrokResultToString(res));
        return res;
    }

    for(std::vector<cv::Mat>::iterator it = faceImages.begin(); it != faceImages.end(); ++it)
    {
        TRACE_T("Recognizing users on new face");
        cv::Mat currentFace = (cv::Mat)*it;
        std::map<std::string, double> currenFaceSimilarities;

        if(FROK_RESULT_SUCCESS != (res = recognizer->SetTargetImage(currentFace)))
        {
            TRACE_F_T("Failed to SetTargetImage on result %s", FrokResultToString(res));
            continue;
        }

        if(FROK_RESULT_SUCCESS != (res = recognizer->GetSimilarityOfFaceWithModels(currenFaceSimilarities)))
        {
            TRACE_F_T("Failed to GetSimilarityOfFaceWithModels on result %s", FrokResultToString(res));
            continue;
        }
        similarities.push_back(currenFaceSimilarities);
    }

    clock_gettime(CLOCK_REALTIME, &endTime);

    printf("Recognition finished\n");
    print_time(startTime, endTime);

    if(similarities.size() == 0)
    {
        TRACE_F_T("Nothing similar to requested users was found on picture");
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    TRACE_T("Recognition finished");

    return FROK_RESULT_SUCCESS;
}

FrokResult TrainUserModel(std::vector<std::string> ids, const char *userBasePath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
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

    if(userBasePath == NULL)
    {
        TRACE_F_T("Invalid parameter: userBasePath = %p", userBasePath);
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
        currentUserFolder.append(*it).append("/");

        std::vector<std::string> userPhotos;

        std::string photosPath = currentUserFolder;
        photosPath.append("photos/");
        if(-1 == getFilesFromDir(photosPath.c_str(), userPhotos))
        {
            TRACE_F_T("Failed to get photos from directory %s", currentUserFolder.c_str());
            continue;
        }

        TRACE_T("Found %u photos for user %s", (unsigned)userPhotos.size(), ((std::string)*it).c_str());

        if(!userPhotos.empty())
        {
            std::vector<cv::Mat> faceImages;

            for(std::vector<std::string>::iterator iterImage = userPhotos.begin(); iterImage != userPhotos.end(); ++iterImage)
            {
                TRACE_S_T("Processing image %s", ((std::string)*iterImage).c_str());
                std::string imageFullPath = photosPath;
                imageFullPath.append((std::string)*iterImage);
                if(FROK_RESULT_SUCCESS != (res = detector->SetTargetImage(imageFullPath.c_str())))
                {
                    TRACE_F_T("Failed to SetTargetImage on result %s", FrokResultToString(res));
                    continue;
                }

                std::vector<cv::Rect> faces;

                if(FROK_RESULT_SUCCESS != (res = detector->GetFacesFromPhoto(faces)))
                {
                    TRACE_F_T("Failed to GetFacesFromPhoto on result %s", FrokResultToString(res));
                    continue;
                }

                if(faces.size() == 1)
                {
                    if(FROK_RESULT_SUCCESS != (res = detector->GetNormalizedFaceImages(faces, faceImages)))
                    {
                        TRACE_F_T("Failed to GetNormalizedFaceImages on result %s", FrokResultToString(res));
                        continue;
                    }

                    TRACE_S_T("Face detection succeed for photo %s", ((std::string)*iterImage).c_str());
                    TRACE_T("Saving result face");
                    std::string saveImagePath = currentUserFolder;
                    saveImagePath.append("faces/").append((std::string)*iterImage);
                    try
                    {
                        cv::imwrite(saveImagePath, faceImages[faceImages.size() - 1]);
                    }
                    catch(...)
                    {
                        TRACE_F_T("Failed to save image %s", ((std::string)*iterImage).c_str());
                    }

                }
                else
                {
                    TRACE_F_T("Invalid number of faces found on photo: %u", (unsigned)faces.size());
                    continue;
                }
            }

            if(!faceImages.empty())
            {
                TRACE_T("Found %u images for user %s. Generating user model", (unsigned)faceImages.size(), ((std::string)*it).c_str());
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

                if(FROK_RESULT_SUCCESS != (res = recognizer->AddFrokUserModel(((std::string)*it), *model)))
                {
                    TRACE_F_T("Failed to AddFrokUserModel on result %s", FrokResultToString(res));
                    continue;
                }
            }
            else
            {
                TRACE_F_T("No faces found for user %s", ((std::string)*it).c_str());
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
