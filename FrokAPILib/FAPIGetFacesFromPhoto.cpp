#include "FrokAPIFunction.h"

#pragma GCC poison IplImage
#define MODULE_NAME     "FROK_API"

// inout parameters
static std::string strInParams [] = {"id", "photoName"};
static std::string strOutParams [] = {"arrObjFaceRects"};
static std::vector<std::string> InGetFacesFromPhotoParameters(strInParams, strInParams + sizeof(strInParams) / sizeof(*strInParams));
static std::vector<std::string> OutGetFacesFromPhotoParameters(strOutParams, strOutParams + sizeof(strOutParams) / sizeof(*strOutParams));

typedef struct
{
    std::string id;
    std::string photoName;
} StructInParams;

typedef struct
{
    std::vector< cv::Rect > faceRects;
} StructOutParams;

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
FrokAPIFunction FAPI_GetFacesFromPhoto(GetFacesFromPhoto, InGetFacesFromPhotoParameters, OutGetFacesFromPhotoParameters, functionDescription,
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

    if(!jsonParams.HasKeys(InGetFacesFromPhotoParameters))
    {
        TRACE_F("Invalid parameter: input json doesn't have all mandatory keys.");
        TRACE_N("Mandatory parameter:");
        for(std::vector<std::string>::const_iterator it = InGetFacesFromPhotoParameters.begin(); it != InGetFacesFromPhotoParameters.end(); ++it)
        {
            TRACE_N("\t%s", ((std::string)*it).c_str());
        }

        TRACE_N("Input json: %s", psConvertParams->jsonParameters.c_str());
        return false;
    }

    StructInParams *funcParameters = new StructInParams;

    funcParameters->id = jsonParams["id"].ToString();
    funcParameters->photoName = jsonParams["photoName"].ToString();

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

    StructOutParams *params = (StructOutParams*)psConvertParams->functionParameters;

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

    jResult["arrObjFaceRects"] = jFoundFaces;

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
        delete (StructOutParams*)psConvertParams->functionParameters;
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
    StructInParams *in = (StructInParams*)inParams;
    *outParams = new StructOutParams;
    FrokResult res;

    std::string imageFullPath = userBasePath;
    imageFullPath.append(in->id).append("/photos/").append(in->photoName);

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
        ((StructOutParams*)*outParams)->faceRects.push_back(face[0]);

        face.clear();
        faceImage.clear();
    }

    if(((StructOutParams*)*outParams)->faceRects.empty())
    {
        TRACE_F_T("No faces found");
        return FROK_RESULT_NOT_A_FACE;
    }

    TRACE_T("Recognition finished");

    return FROK_RESULT_SUCCESS;
}
