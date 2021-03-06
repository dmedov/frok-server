#include "FrokAPIFunction.h"


#pragma GCC poison IplImage
#define MODULE_NAME     "FROK_API"

// inout parameters
static std::string strInAddFaceFromPhotoToModelParams [] = {"userId", "phName", "faceNum"};
static std::string strOutAddFaceFromPhotoToModelParams [] = {};
static std::vector<std::string> inAddFaceFromPhotoToModelParams(strInAddFaceFromPhotoToModelParams, strInAddFaceFromPhotoToModelParams + sizeof(strInAddFaceFromPhotoToModelParams) / sizeof(*strInAddFaceFromPhotoToModelParams));
static std::vector<std::string> outAddFaceFromPhotoToModelParams(strOutAddFaceFromPhotoToModelParams, strOutAddFaceFromPhotoToModelParams + sizeof(strOutAddFaceFromPhotoToModelParams) / sizeof(*strOutAddFaceFromPhotoToModelParams));

typedef struct
{
    std::string userId;
    std::string photoName;
    int faceNumber;
} StructInAddFaceFromPhotoToModelParams;

typedef struct {} StructOutAddFaceFromPhotoToModelParams;

// function description
const char functionDescription [] = "This function Recognize users given in parameters at specified photo";
const char parametersDescription [] = "\"arrIds\": \"[in] array of users` for whom database would be created\", \
        \"photoName\": \"[in] target photo (the one users will be recognized on)\", \
        \"arrMapSimilarities\": \"[out] probabilities of friend with userId to be the human on the photo. Probabilities\
        are given for all found faces\"";

// timeout
static unsigned long int timeout = 300;        //300 sec

// Function and parameters convertors
FrokResult AddFaceFromPhotoToModel(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
bool FAPI_AddFaceFromPhotoToModel_JSON2FUNCP(ConvertParams* converterParams);
bool FAPI_AddFaceFromPhotoToModel_FUNCP2JSON(ConvertParams* converterParams);

// FAPI object
FrokAPIFunction FAPI_AddFaceFromPhotoToModel(AddFaceFromPhotoToModel, outAddFaceFromPhotoToModelParams, outAddFaceFromPhotoToModelParams, functionDescription,
                parametersDescription, timeout, FAPI_AddFaceFromPhotoToModel_JSON2FUNCP,
                FAPI_AddFaceFromPhotoToModel_FUNCP2JSON);

bool FAPI_AddFaceFromPhotoToModel_JSON2FUNCP(ConvertParams* psConvertParams)
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

    if(!jsonParams.HasKeys(inAddFaceFromPhotoToModelParams))
    {
        TRACE_F("Invalid parameter: input json doesn't have all mandatory keys.");
        TRACE_N("Mandatory parameter:");
        for(std::vector<std::string>::const_iterator it = inAddFaceFromPhotoToModelParams.begin(); it != inAddFaceFromPhotoToModelParams.end(); ++it)
        {
            TRACE_N("\t%s", ((std::string)*it).c_str());
        }

        TRACE_N("Input json: %s", psConvertParams->jsonParameters.c_str());
        return false;
    }

    StructInAddFaceFromPhotoToModelParams *funcParameters = new StructInAddFaceFromPhotoToModelParams;
    funcParameters->userId = jsonParams["userId"].ToString();
    funcParameters->photoName = jsonParams["phName"].ToString();
    funcParameters->faceNumber = atoi(jsonParams["faceNum"].ToString().c_str());

    psConvertParams->functionParameters = funcParameters;

    return true;
}

bool FAPI_AddFaceFromPhotoToModel_FUNCP2JSON(ConvertParams* psConvertParams)
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
        delete [] (StructInAddFaceFromPhotoToModelParams*)psConvertParams->functionParameters;
        psConvertParams->functionParameters = NULL;
    }
    return true;
}

FrokResult AddFaceFromPhotoToModel(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
{
    if(inParams == NULL || outParams == NULL || userBasePath == NULL || targetPhotosPath == NULL ||
            detector == NULL || recognizer == NULL)
    {
        TRACE_F("Invalid parameters. inParams = %p, outParams = %p, userBasePath = %p,\
                targetPhotosPath = %p, detector = %p, recognizer = %p",
                inParams, outParams, userBasePath, targetPhotosPath, detector, recognizer);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    StructInAddFaceFromPhotoToModelParams *in = (StructInAddFaceFromPhotoToModelParams*)inParams;
    *outParams = NULL;

    TRACE_T("AddFaceFromPhotoToModel started");

    FrokResult res;

    std::string currentUserFolder = userBasePath;
    currentUserFolder.append(in->userId).append("/");

    std::vector<cv::Mat> faceImage;
    std::vector<cv::Rect> faces;
    std::vector<cv::Rect> face;
    TRACE_S_T("Processing image %s", in->photoName.c_str());

    std::string imageFullPath = currentUserFolder;
    imageFullPath.append("photos/").append(in->photoName);

    if(FROK_RESULT_SUCCESS != (res = detector->SetTargetImage(imageFullPath.c_str(), true)))
    {
        TRACE_F_T("Failed to SetTargetImage on result %s", FrokResultToString(res));
        return res;
    }

    if(FROK_RESULT_SUCCESS != (res = detector->GetFacesFromPhoto(faces)))
    {
        TRACE_F_T("Failed to GetFacesFromPhoto on result %s", FrokResultToString(res));
        return res;
    }

    if(faces.size() < (size_t)in->faceNumber)
    {
        TRACE_F("Invalid faceNumber. Found %zu faces, faceNumber = %d", faces.size(), in->faceNumber);
        return FROK_RESULT_INVALID_PARAMETER;
    }

    face.push_back(faces[in->faceNumber]);

    if(FROK_RESULT_SUCCESS != (res = detector->GetNormalizedFaceImages(face, faceImage)))
    {
        TRACE_F_T("Failed to GetNormalizedFaceImages on result %s", FrokResultToString(res));
        return res;
    }

    TRACE_S_T("Face detection succeed for photo %s", in->photoName.c_str());

    TRACE_T("Saving result face");
    std::stringstream faceNumberStr;
    faceNumberStr << in->faceNumber;
    std::string saveImagePath = currentUserFolder;
    saveImagePath.append("faces/").append(in->photoName).append("_").append(faceNumberStr.str());
    try
    {
        cv::imwrite(saveImagePath, faceImage[0]);
    }
    catch(...)
    {
        TRACE_F_T("Failed to save image %s", in->photoName.c_str());
        return FROK_RESULT_OPENCV_ERROR;
    }

    TRACE_T("AddFaceFromPhotoToModel finished");

    return FROK_RESULT_SUCCESS;
}

