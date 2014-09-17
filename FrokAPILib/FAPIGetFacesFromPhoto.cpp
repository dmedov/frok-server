#include "FrokAPIFunction.h"

#pragma GCC poison IplImage
#define MODULE_NAME     "FROK_API"

// inout parameters
static std::string strInGetFacesFromPhotoParams [] = {"userId", "phName"};
static std::string strOutGetFacesFromPhotoParams [] = {"resFaceCoords"};
static std::vector<std::string> inGetFacesFromPhotoParams(strInGetFacesFromPhotoParams, strInGetFacesFromPhotoParams + sizeof(strInGetFacesFromPhotoParams) / sizeof(*strInGetFacesFromPhotoParams));
static std::vector<std::string> outGetFacesFromPhotoParams(strOutGetFacesFromPhotoParams, strOutGetFacesFromPhotoParams + sizeof(strOutGetFacesFromPhotoParams) / sizeof(*strOutGetFacesFromPhotoParams));

typedef struct
{
    std::string userId;
    std::string photoName;
} StructInGetFacesFromPhotoParams;

typedef struct
{
    std::vector< cv::Rect > faceRects;
} StructOutGetFacesFromPhotoParams;

// function description
const char functionDescription [] = "This function Recognize users given in parameters at specified photo";
const char parametersDescription [] = "\"arrIds\": \"[in] array of users` for whom database would be created\", \
        \"photoName\": \"[in] target photo (the one users will be recognized on)\", \
        \"arrMapSimilarities\": \"[out] probabilities of friend with userId to be the human on the photo. Probabilities\
        are given for all found faces\"";

// timeout
static unsigned long int timeout = 300;        //300 sec

// Function and parameters convertors
FrokResult GetFacesFromPhoto(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
bool FAPI_GetFacesFromPhoto_JSON2FUNCP(ConvertParams* converterParams);
bool FAPI_GetFacesFromPhoto_FUNCP2JSON(ConvertParams* converterParams);

// FAPI object
FrokAPIFunction FAPI_GetFacesFromPhoto(GetFacesFromPhoto, inGetFacesFromPhotoParams, outGetFacesFromPhotoParams, functionDescription,
                parametersDescription, timeout, FAPI_GetFacesFromPhoto_JSON2FUNCP,
                FAPI_GetFacesFromPhoto_FUNCP2JSON);

bool FAPI_GetFacesFromPhoto_JSON2FUNCP(ConvertParams* psConvertParams)
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

    if(!jsonParams.HasKeys(inGetFacesFromPhotoParams))
    {
        TRACE_F("Invalid parameter: input json doesn't have all mandatory keys.");
        TRACE_N("Mandatory parameter:");
        for(std::vector<std::string>::const_iterator it = inGetFacesFromPhotoParams.begin(); it != inGetFacesFromPhotoParams.end(); ++it)
        {
            TRACE_N("\t%s", ((std::string)*it).c_str());
        }

        TRACE_N("Input json: %s", psConvertParams->jsonParameters.c_str());
        return false;
    }

    StructInGetFacesFromPhotoParams *funcParameters = new StructInGetFacesFromPhotoParams;

    funcParameters->userId = jsonParams["userId"].ToString();
    funcParameters->photoName = jsonParams["phName"].ToString();

    psConvertParams->functionParameters = funcParameters;

    return true;
}

bool FAPI_GetFacesFromPhoto_FUNCP2JSON(ConvertParams* psConvertParams)
{
    if(!psConvertParams)
    {
        TRACE_F_T("Invalid parameter: converterParams = %p", psConvertParams);
        return false;
    }

    psConvertParams->jsonParameters.clear();

    StructOutGetFacesFromPhotoParams *params = (StructOutGetFacesFromPhotoParams*)psConvertParams->functionParameters;

    json::Object jResult;
    json::Array jFoundFaces;

    for(std::vector<cv::Rect>::iterator it = params->faceRects.begin(); it != params->faceRects.end(); ++it)
    {
        json::Object jFaceRect;
        jFaceRect["x1"] = it->x;
        jFaceRect["y1"] = it->y;
        jFaceRect["x2"] = it->x + it->width;
        jFaceRect["y2"] = it->y + it->height;

        jFoundFaces.push_back(jFaceRect);
    }

    jResult["resFaceCoords"] = jFoundFaces;

    try
    {
        psConvertParams->jsonParameters = json::Serialize(jResult);
    }
    catch(...)
    {
        TRACE_F("Failed to serialize result json");
        return false;
    }

    if(psConvertParams->functionParameters != NULL)
    {
        delete (StructOutGetFacesFromPhotoParams*)psConvertParams->functionParameters;
        psConvertParams->functionParameters = NULL;
    }
    return true;
}

FrokResult GetFacesFromPhoto(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
{
    if(inParams == NULL || outParams == NULL || userBasePath == NULL || targetPhotosPath == NULL ||
            detector == NULL || recognizer == NULL)
    {
        TRACE_F("Invalid parameters. inParams = %p, outParams = %p, userBasePath = %p,\
                targetPhotosPath = %p, detector = %p, recognizer = %p",
                inParams, outParams, userBasePath, targetPhotosPath, detector, recognizer);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    StructInGetFacesFromPhotoParams *in = (StructInGetFacesFromPhotoParams*)inParams;
    *outParams = new StructOutGetFacesFromPhotoParams;
    FrokResult res;

    std::string imageFullPath = userBasePath;
    imageFullPath.append(in->userId).append("/photos/").append(in->photoName);

    if(FROK_RESULT_SUCCESS != (res = detector->SetTargetImage(imageFullPath.c_str(), true)))
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

    for(std::vector<cv::Rect>::iterator it = faces.begin(); it != faces.end(); ++it)
    {
        std::vector<cv::Rect> face;
        std::vector<cv::Mat> faceImage;

        face.push_back(*it);

        if(FROK_RESULT_SUCCESS != (res = detector->GetNormalizedFaceImages(face, faceImage)))
        {
            TRACE_W_T("Failed to GetNormalizedFaceImages on result %s", FrokResultToString(res));
            continue;
        }

        // Yeah it is face for real. Kappa.
        TRACE_S("Face found: x1 = %d, y1 = %d, x2 = %d. y2 = %d", ((cv::Rect)face[0]).x, ((cv::Rect)face[0]).y, ((cv::Rect)face[0]).x + ((cv::Rect)face[0]).width, ((cv::Rect)face[0]).y + ((cv::Rect)face[0]).height);
        ((StructOutGetFacesFromPhotoParams*)*outParams)->faceRects.push_back(face[0]);

        face.clear();
        faceImage.clear();
    }

    if(((StructOutGetFacesFromPhotoParams*)*outParams)->faceRects.empty())
    {
        TRACE_F_T("No faces found");
        return FROK_RESULT_NOT_A_FACE;
    }

    TRACE_T("Recognition finished");

    return FROK_RESULT_SUCCESS;
}
