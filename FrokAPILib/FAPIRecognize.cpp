#include "stdlib.h"

#include "FrokAPIFunction.h"

#pragma GCC poison IplImage
#define MODULE_NAME     "FROK_API"

// inout parameters
static std::string strInParams [] = {"arrUserIds", "photoName"};
static std::string strOutParams [] = {"arrOfFacesCoordsWithUserIdAndSimilarity"};
static std::vector<std::string> InRecognizeParameters(strInParams, strInParams + sizeof(strInParams) / sizeof(*strInParams));
static std::vector<std::string> OutRecognizeParameters(strOutParams, strOutParams + sizeof(strOutParams) / sizeof(*strOutParams));

typedef struct
{
    std::vector<std::string> ids;
    std::string photoName;
} StructInParams;

typedef struct
{
    std::vector<  cv::Rect > coords;
    std::vector< std::map <std::string, double> > similarities;
} StructOutParams;

// function description
const char functionDescription [] = "This function Recognize users given in parameters at specified photo";
const char parametersDescription [] = "\"arrUserIds\": \"[in] array of users` for whom database would be created\", \
        \"photoName\": \"[in] target photo (the one users will be recognized on)\", \
        \"arrMapSimilarities\": \"[out] probabilities of friend with userId to be the human on the photo. Probabilities\
        are given for all found faces\"";

// timeout
static unsigned long int timeout = 300;        //300 sec

// Function and parameters convertors
FrokResult Recognize(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
bool FAPI_Recognize_JSON2FUNCP(ConvertParams* converterParams);
bool FAPI_Recognize_FUNCP2JSON(ConvertParams* converterParams);

// FAPI object
FrokAPIFunction FAPI_Recognize(Recognize, InRecognizeParameters, OutRecognizeParameters, functionDescription,
                parametersDescription, timeout, FAPI_Recognize_JSON2FUNCP,
                FAPI_Recognize_FUNCP2JSON);

bool FAPI_Recognize_JSON2FUNCP(ConvertParams* psConvertParams)
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

    if(!jsonParams.HasKeys(InRecognizeParameters))
    {
        TRACE_F("Invalid parameter: input json doesn't have all mandatory keys.");
        TRACE_N("Mandatory parameter:");
        for(std::vector<std::string>::const_iterator it = InRecognizeParameters.begin(); it != InRecognizeParameters.end(); ++it)
        {
            TRACE_N("\t%s", ((std::string)*it).c_str());
        }

        TRACE_N("Input json: %s", psConvertParams->jsonParameters.c_str());
        return false;
    }

    StructInParams *funcParameters = new StructInParams;
    json::Array arrUserIds = jsonParams["arrUserIds"].ToArray();

    for (unsigned i = 0; i < arrUserIds.size(); i++)
    {
        funcParameters->ids.push_back(arrUserIds[i].ToString());
    }
    funcParameters->photoName = jsonParams["photoName"].ToString();

    psConvertParams->functionParameters = funcParameters;

    return true;
}

bool FAPI_Recognize_FUNCP2JSON(ConvertParams* psConvertParams)
{
    if(!psConvertParams)
    {
        TRACE_F_T("Invalid parameter: converterParams = %p", psConvertParams);
        return false;
    }

    psConvertParams->jsonParameters.clear();

    StructOutParams *params = (StructOutParams*)psConvertParams->functionParameters;

    unsigned i = 0;
    json::Object resultJson;
    json::Array jsonArrOfFacesCoordsWithUserIdAndSimilarity;

    for(std::vector< std::map <std::string, double> > ::iterator faceIterator = params->similarities.begin();
        faceIterator != params->similarities.end(); ++faceIterator)
    {
        cv::Rect faceCoords = params->coords.at(i++);
        std::map<std::string, double> currentFace = (std::map<std::string, double>)*faceIterator;

        json::Array jsonUsersProbability;
        for (std::map<std::string, double>::iterator userIterator = currentFace.begin();
             userIterator != currentFace.end(); ++userIterator)
        {
            json::Object jsonFace;
            std::stringstream doubleToString;
            doubleToString << (double)userIterator->second;
            jsonFace["userId"] = userIterator->first;
            jsonFace["probability"] = doubleToString.str();
            jsonUsersProbability.push_back(jsonFace);
        }

        json::Object jFaceCoordsAndUsersOnPhoto;

        json::Object jFaceRect;
        jFaceRect["x1"] = faceCoords.x;
        jFaceRect["y1"] = faceCoords.y;
        jFaceRect["x2"] = faceCoords.x + faceCoords.width;
        jFaceRect["y2"] = faceCoords.y + faceCoords.height;

        jFaceCoordsAndUsersOnPhoto["coords"] = jFaceRect;
        jFaceCoordsAndUsersOnPhoto["arrUserIds"] = jsonUsersProbability;

        jsonArrOfFacesCoordsWithUserIdAndSimilarity.push_back(jFaceCoordsAndUsersOnPhoto);
    }

    resultJson["arrOfFacesCoordsWithUserIdAndSimilarity"] = jsonArrOfFacesCoordsWithUserIdAndSimilarity;
    try
    {
        psConvertParams->jsonParameters = json::Serialize(resultJson);
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

FrokResult Recognize(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
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

    std::vector<std::string> ids = in->ids;
    std::string photoName = in->photoName;

    ((StructOutParams*)*outParams)->similarities.clear();
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

    if(FROK_RESULT_SUCCESS != (res = detector->SetTargetImage(targetImageFullPath.c_str(), true)))
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

    int i = 0;
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
        ((StructOutParams*)*outParams)->coords.push_back(faces.at(i));
        ((StructOutParams*)*outParams)->similarities.push_back(currenFaceSimilarities);
    }

    if(((StructOutParams*)*outParams)->similarities.empty())
    {
        TRACE_F_T("Nothing similar to requested users was found on picture");
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    TRACE_T("Recognition finished");

    return FROK_RESULT_SUCCESS;
}

