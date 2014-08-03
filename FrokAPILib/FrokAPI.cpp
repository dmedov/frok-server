#include "FrokAPI.h"

#define MODULE_NAME "FAPI"

FrokAPI::FrokAPI(const char *photo_base_path, const char *targets_folder_path, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
{
    this->photo_base_path = new char [strlen(photo_base_path) + 1];
    this->targets_folder_path = new char [strlen(targets_folder_path) + 1];
    strcpy(this->photo_base_path, photo_base_path);
    strcpy(this->targets_folder_path, targets_folder_path);
    this->detector = detector;
    this->recognizer = recognizer;
}

FrokAPI::FrokAPI(FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer)
{
    this->photo_base_path = new char [strlen(DEFAULT_PHOTO_BASE_PATH) + 1];
    this->targets_folder_path = new char [strlen(DEFAULT_TARGETS_FOLDER_PATH) + 1];
    strcpy(this->photo_base_path, DEFAULT_PHOTO_BASE_PATH);
    strcpy(this->targets_folder_path, DEFAULT_TARGETS_FOLDER_PATH);
    this->detector = detector;
    this->recognizer = recognizer;
}

FrokAPI::~FrokAPI()
{
    delete []photo_base_path;
    delete []targets_folder_path;
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
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }
    FrokAPIFunction *function = functions[functionName];
    ConvertParams inParams;
    ConvertParams outParams;
    inParams.jsonParameters = inJson;
    function->ConvertJsonToFunctionParameters(&inParams);
    FrokResult res = function->function(inParams.functionParameters, &outParams.functionParameters,
                                        photo_base_path, targets_folder_path, detector, recognizer);
    function->ConvertFunctionReturnToJson(&outParams);
    outJson = outParams.jsonParameters;
    return res;
}
