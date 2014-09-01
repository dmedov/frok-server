#include "FrokAPIFunction.h"

#pragma GCC poison IplImage
#define MODULE_NAME     "FROK_API"

// inout parameters
static std::string strInParams [] = {"arrIds"};
static std::string strOutParams [] = {};
static std::vector<std::string> InTrainUserModelParameters(strInParams, strInParams + sizeof(strInParams) / sizeof(*strInParams));
static std::vector<std::string> OutTrainUserModelParameters(strOutParams, strOutParams + sizeof(strOutParams) / sizeof(*strOutParams));

typedef struct
{
    std::vector<std::string> ids;
} StructInParams;

// function description
const char functionDescription [] = "This function generates user's grey faces' database that will be used in \
        rocignition process";
const char parametersDescription [] = "arrIds: [in] array of users' for whom database would be created";

// timeout
static unsigned long int timeout = 300;        //300 sec

// Function and parameters convertors
FrokResult TrainUserModel(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
bool FAPI_TrainUserModel_JSON2FUNCP(ConvertParams* converterParams);
bool FAPI_TrainUserModel_FUNCP2JSON(ConvertParams* converterParams);

// FAPI object
FrokAPIFunction FAPI_TrainUserModel(TrainUserModel, InTrainUserModelParameters, OutTrainUserModelParameters, functionDescription,
                parametersDescription, timeout, FAPI_TrainUserModel_JSON2FUNCP,
                FAPI_TrainUserModel_FUNCP2JSON);

bool FAPI_TrainUserModel_JSON2FUNCP(ConvertParams* psConvertParams)
{
    if(!psConvertParams)
    {
        TRACE_F("Invalid parameter: converterParams = %p", psConvertParams);
        return false;
    }

    json::Object jsonParams;
    try
    {
         jsonParams = (json::Object)json::Deserialize(psConvertParams->jsonParameters);
    }
    catch(...)
    {
        TRACE_F("Failed to deserialize input json %s", psConvertParams->jsonParameters.c_str());
        return false;
    }

    if(!jsonParams.HasKeys(InTrainUserModelParameters))
    {
        TRACE_F("Invalid parameter: input json doesn't have all mandatory keys.");
        TRACE("Mandatory parameter:");
        for(std::vector<std::string>::const_iterator it = InTrainUserModelParameters.begin(); it != InTrainUserModelParameters.end(); ++it)
        {
            TRACE("\t%s", ((std::string)*it).c_str());
        }

        TRACE("Input json: %s", psConvertParams->jsonParameters.c_str());
        return false;
    }

    StructInParams *funcParameters = new StructInParams;
    json::Array arrIds = jsonParams["arrIds"].ToArray();

    for (unsigned i = 0; i < arrIds.size(); i++)
    {
        funcParameters->ids.push_back(arrIds[i].ToString());
    }

    psConvertParams->functionParameters = funcParameters;

    return true;
}

bool FAPI_TrainUserModel_FUNCP2JSON(ConvertParams* psConvertParams)
{
    if(!psConvertParams)
    {
        TRACE_F_T("Invalid parameter: converterParams = %p", psConvertParams);
        return false;
    }

    psConvertParams->jsonParameters.clear();
    psConvertParams->jsonParameters = "{}";
    if(psConvertParams->functionParameters != NULL)
    {
        delete [] (StructInParams*)psConvertParams->functionParameters;
        psConvertParams->functionParameters = NULL;
    }
    return true;
}

FrokResult TrainUserModel(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
{
    if(inParams == NULL || outParams == NULL || userBasePath == NULL || targetPhotosPath == NULL ||
            detector == NULL || recognizer == NULL)
    {
        TRACE_F("Invalid parameters. inParams = %p, outParams = %p, userBasePath = %p,\
                targetPhotosPath = %p, detector = %p, recognizer = %p",
                inParams, outParams, userBasePath, targetPhotosPath, detector, recognizer);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    StructInParams *in = (StructInParams*)inParams;
    *outParams = NULL;

    std::vector<std::string> ids = in->ids;
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
        std::string currentUserFolder = userBasePath;
        currentUserFolder.append(*it).append("/");

        std::vector<std::string> userPhotos;

        std::string photosPath = currentUserFolder;
        photosPath.append("photos/");
        char **files;
        unsigned filesNum = 0;
        if(-1 == getFilesFromDir(photosPath.c_str(), &files, &filesNum))
        {
            TRACE_F_T("Failed to get photos from directory %s", currentUserFolder.c_str());
            continue;
        }

        for(unsigned i = 0; i < filesNum; i++)
        {
            userPhotos.push_back(files[i]);
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
                if(FROK_RESULT_SUCCESS != (res = detector->SetTargetImage(imageFullPath.c_str(), true)))
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
                FaceModelAbstract *model;
                try
                {
                    model = new FaceModelEigenfaces((std::string)*it);
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

                if(FROK_RESULT_SUCCESS != (res = recognizer->AddFaceUserModel(((std::string)*it), model)))
                {
                    TRACE_F_T("Failed to AddFaceUserModel on result %s", FrokResultToString(res));
                    continue;
                }
            }
            else
            {
                TRACE_F_T("No faces found for user %s", ((std::string)*it).c_str());
                isSuccess = false;
            }
        }
    }

    if(isSuccess == false)
    {
        TRACE_F_T("Training failed for one or more user - return failure");
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    TRACE_T("Training finished");

    return FROK_RESULT_SUCCESS;
}
