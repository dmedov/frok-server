#include "FrokAPI.h"
#include "json.h"

#include <sys/time.h>
#include <time.h>
#include <sys/times.h>

#define MODULE_NAME "FAPI"

extern "C"
{
void *frokAPIAlloc(const char *photo_base_path, const char *targets_folder_path,
                   void *detector, void *recognizer)
{
    if((photo_base_path == NULL) || (targets_folder_path == NULL))
    {
        TRACE_F("Invalid parameter: photo_base_path = %p, targets_folder_path = %p", photo_base_path, targets_folder_path);
        return NULL;
    }

    FrokAPI *instance = NULL;
    try
    {
        instance = new FrokAPI(photo_base_path, targets_folder_path, (FaceDetectorAbstract*)detector, (FaceRecognizerAbstract*)recognizer);
    }
    catch(FrokResult error)
    {
        TRACE_F("Failed to create FrokAPI instance on result %s", FrokResultToString(error));
        return NULL;
    }
    return instance;
}

void frokAPIDealloc(void *instance)
{
    if(instance == NULL)
        return;

    delete (FrokAPI*)instance;
    instance = NULL;
}

FrokResult frokAPIExecuteFunction(void *instance, const char *functionName, const char *inJson, char **outJson)
{
    std::string strFunctionName = functionName;
    std::string strInJson = inJson;
    std::string strOutJson;
    json::Object jOutJson;
    json::Object jInJson;
    FrokResult res;
    if((instance == NULL) || (functionName = NULL) || (inJson == NULL) || (outJson == NULL))
    {
        TRACE_F("Invalid parameters: instance = %p, function name = %p, inJson = %p, outJson = %p", instance, functionName, inJson, outJson);
        jOutJson["result"] = "fail";
        jOutJson["reason"] = "internal error";
        try
        {
            jInJson = json::Deserialize(strInJson);
            jOutJson["reqId"] = jInJson["reqId"];
        }
        catch(...)
        {
            jOutJson["reply_sock"] = -1;
            jOutJson["req_id"] = -1;
        }
        res = FROK_RESULT_NULL;
    }
    else
    {
        try
        {
            res = ((FrokAPI*)instance)->ExecuteFunction(strFunctionName, strInJson, strOutJson);
        }
        catch(FrokResult result)
        {
            TRACE_F("ExecuteFunction failed. Exception captured");
            res = result;
        }
        catch(...)
        {
            TRACE_F("ExecuteFunction failed. Exception captured");
            res = FROK_RESULT_UNSPECIFIED_ERROR;
        }

        try
        {
            jOutJson = json::Deserialize(strOutJson);
            try
            {
                jInJson = json::Deserialize(strInJson);
                jOutJson["reqId"] = jInJson["reqId"];
            }
            catch(...)
            {
                jOutJson["reqid"] = -1;
            }

            if(res != FROK_RESULT_SUCCESS)
            {
                jOutJson["result"] = "fail";
                jOutJson["reason"] = FrokResultToString(res);
            }
            else
            {
                jOutJson["result"] = "success";
            }
        }
        catch(...)
        {
            jOutJson["result"] = "fail";
            jOutJson["reason"] = "internal error";
        }
    }

    std::string str = json::Serialize(jOutJson);
    *outJson = new char [str.size() + 2];
    strcpy(*outJson, str.c_str());
    strcat(*outJson, "\n\0");

    return res;
}

char *getFunctionFromJson(const char *inJson)
{
    if(inJson == NULL)
    {
        TRACE_F("Invalid parameter: inJson = %p", inJson);
        return NULL;
    }
    std::string strJson = inJson;
    std::string cmd;
    json::Object jObject;
    try
    {
         jObject = json::Deserialize(strJson);
         cmd = jObject["cmd"].ToString();
    }
    catch(...)
    {
        TRACE_F("Invalid parameter. String %s is not a json", inJson);
        return NULL;
    }

    char *result = new char [cmd.size() + 1];
    strcpy(result, cmd.c_str());
    return result;
}

void frokAPIInit(void *instance)
{
    if(instance == NULL)
    {
        TRACE_F("Invalid parameter: instance = %p", instance);
        return;
    }
    ((FrokAPI*)instance)->AddAPIFunction("addFace", &FAPI_AddFaceFromPhotoToModel);
    ((FrokAPI*)instance)->AddAPIFunction("getFaces", &FAPI_GetFacesFromPhoto);
    ((FrokAPI*)instance)->AddAPIFunction("train", &FAPI_TrainUserModel);
    ((FrokAPI*)instance)->AddAPIFunction("recognize", &FAPI_Recognize);
}

void frokAPIDeinit(void *UNUSED(instance))
{
    return;
}
}

FrokAPI::FrokAPI(const char *photo_base_path, const char *targets_folder_path, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
{
    if((photo_base_path == NULL) || (targets_folder_path == NULL))
    {
        TRACE_F("Invalid parameter: photo_base_path = %p, targets_folder_path = %p", photo_base_path, targets_folder_path);
        throw FROK_RESULT_INVALID_PARAMETER;
    }
    this->photo_base_path = new char [strlen(photo_base_path) + 1];
    this->targets_folder_path = new char [strlen(targets_folder_path) + 1];
    strcpy(this->photo_base_path, photo_base_path);
    strcpy(this->targets_folder_path, targets_folder_path);
    this->detector = detector;
    this->recognizer = recognizer;
}

FrokAPI::~FrokAPI()
{
    delete []photo_base_path;
    delete []targets_folder_path;
    photo_base_path = NULL;
    targets_folder_path = NULL;
}

void FrokAPI::AddAPIFunction(std::string functionName, FrokAPIFunction *function)
{
    functions[functionName] = function;
}

void FrokAPI::GetAvailableFunctions(std::vector<std::string> &availableFunctions)
{
    availableFunctions.clear();
    for(std::map<std::string, FrokAPIFunction*>::iterator it = functions.begin(); it != functions.end(); ++it)
    {
        availableFunctions.push_back(it->first);
    }
}

FrokResult FrokAPI::ExecuteFunction(std::string functionName, std::string inJson, std::string &outJson)
{
    if(functions.find(functionName) == functions.end())
    {
        TRACE_F("No such function \"%s\"", functionName.c_str());
        return FROK_RESULT_INVALID_PARAMETER;
    }
    FrokAPIFunction *function = functions[functionName];
    ConvertParams inParams;
    ConvertParams outParams;
    inParams.jsonParameters = inJson;
    if(!function->ConvertJsonToFunctionParameters(&inParams))
    {
        TRACE_F("ConvertJsonToFunctionParameters failed");
        return FROK_RESULT_INVALID_PARAMETER;
    }

    bool ts_fail = false;
    struct timespec start_ts, end_ts;

    memset(&start_ts, 0, sizeof(struct timespec));
    memset(&end_ts, 0, sizeof(struct timespec));

    if(-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_ts))
    {
        ts_fail = true;
    }

    FrokResult res = function->function(inParams.functionParameters, &outParams.functionParameters,
                                        photo_base_path, targets_folder_path, detector, recognizer);

    if(-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_ts))
    {
        ts_fail = true;
    }

    time_t seconds;
    time_t nanoseconds;

    seconds = end_ts.tv_sec - start_ts.tv_sec;

    if(end_ts.tv_nsec >= start_ts.tv_nsec)
    {
        nanoseconds = end_ts.tv_nsec - start_ts.tv_nsec;
    }
    else
    {
        nanoseconds = 1e9 - (end_ts.tv_nsec - start_ts.tv_nsec);
        seconds++;
    }

    if(ts_fail == false)
        TRACE_R_T("Function %s finished. Time elapsed %li.%0li s", functionName.c_str(), seconds, nanoseconds);

    if(!function->ConvertFunctionReturnToJson(&outParams))
    {
        TRACE_F("ConvertFunctionReturnToJson failed");
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }
    outJson = outParams.jsonParameters;
    return res;
}
