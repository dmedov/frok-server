#include "stdlib.h"

#include "FrokAPIFunction.h"

#pragma GCC poison IplImage
#define MODULE_NAME     "FROK_API"

// inout parameters
std::string strInParams [] = {"arrIds", "photoName"};
std::string strOutParams [] = {"arrMapSimilarities"};
std::vector<std::string> InParameters(strInParams, strInParams + sizeof(strInParams) / sizeof(*strInParams));
std::vector<std::string> OutParameters(strOutParams, strOutParams + sizeof(strOutParams) / sizeof(*strOutParams));

typedef struct
{
    std::vector<std::string> ids;
    std::string photoName;
} StructInParams;

typedef struct
{
    std::vector< std::map<std::string, double> > similarities;
} StructOutParams;

// function description
const char functionDescription [] = "This function Recognize users given in parameters at specified photo";
const char parametersDescription [] = "\"arrIds\": \"[in] array of users` for whom database would be created\", \
        \"photoName\": \"[in] target photo (the one users will be recognized on)\", \
        \"arrMapSimilarities\": \"[out] probabilities of friend with userId to be the human on the photo. Probabilities\
        are given for all found faces\"";

// timeout
unsigned long int timeout = 300;        //300 sec

// Function and parameters convertors
FrokResult Recognize(void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
bool FAPI_Recognize_JSON2FUNCP(ConvertParams* converterParams);
bool FAPI_Recognize_FUNCP2JSON(ConvertParams* converterParams);

// FAPI object
FrokAPIFunction FAPI_Recognize(Recognize, InParameters, OutParameters, functionDescription,
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
         jsonParams = json::Deserialize(psConvertParams->jsonParameters);
    }
    catch(...)
    {
        TRACE_F("Failed to deserialize input json %s", psConvertParams->jsonParameters.c_str());
        return false;
    }

    if(!jsonParams.HasKeys(InParameters))
    {
        TRACE_F("Invalid parameter: input json doesn't have all mandatory keys.");
        TRACE("Mandatory parameter:");
        for(std::vector<std::string>::const_iterator it = InParameters.begin(); it != InParameters.end(); ++it)
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

    for(std::vector< std::map<std::string, double> >::iterator faceIterator = params->similarities.begin();
        faceIterator != params->similarities.end(); ++faceIterator)
    {
        std::map<std::string, double> currentFace = (std::map<std::string, double>)*faceIterator;
        json::Object jsonFace;
        for (std::map<std::string, double>::iterator userIterator = currentFace.begin();
             userIterator != currentFace.end(); ++userIterator)
        {
            std::stringstream doubleToString;
            doubleToString << userIterator->second;
            jsonFace[userIterator->first] = doubleToString.str();
            //std::cout << it->first << " => " << it->second << '\n';
        }
        std::stringstream intToString;
        intToString << i++;
        resultJson[intToString.str()] = jsonFace;
    }

    if(psConvertParams->functionParameters != NULL)
    {
        delete [] (StructInParams*)psConvertParams->functionParameters;
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

    *outParams = NULL;
    timespec startTime;
    timespec endTime;

    std::vector<std::string> ids = in->ids;
    std::string photoName = in->photoName;

    TRACE_T("Recognizing started");

    ((StructOutParams*)outParams)->similarities.clear();
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
        ((StructOutParams*)outParams)->similarities.push_back(currenFaceSimilarities);
    }

    clock_gettime(CLOCK_REALTIME, &endTime);

    printf("Recognition finished\n");
    print_time(startTime, endTime);

    if(((StructOutParams*)outParams)->similarities.size() == 0)
    {
        TRACE_F_T("Nothing similar to requested users was found on picture");
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    TRACE_T("Recognition finished");

    return FROK_RESULT_SUCCESS;
}

