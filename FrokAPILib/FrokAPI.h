#ifndef FROKAPI_H
#define FROKAPI_H

// include dependencies
#include "FrokAPIFunction.h"
#include "frokLibCommon.h"

#ifdef __cplusplus

class FrokAPI
{
private:
    FaceDetectorAbstract *detector;
    FaceRecognizerAbstract *recognizer;
    char *photo_base_path;
    char *targets_folder_path;
    std::map<std::string, FrokAPIFunction*> functions;
public:
    FrokAPI(const char *photo_base_path, const char *targets_folder_path,
            FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
    ~FrokAPI();
    void AddAPIFunction(std::string functionName, FrokAPIFunction *function);
    FrokResult ExecuteFunction(std::string functionName, std::string inJson, std::string &outJson);

    void GetAvailableFunctions(std::vector<std::string> &availableFunctions);
};

extern FrokAPI FROK_API_FUNCTIONS[];
#else
#define FrokAPI void *

// returns allocated in memory object
FrokAPI *frokAPIAlloc(const char *photo_base_path, const char *targets_folder_path,
                   void *detector, void *recognizer);
// Adds all FrokAPIFunctions to instance
void frokAPIInit(FrokAPI *instance);

// Verifies json and returns function name
char *getFunctionFromJson(const char *json);

// Executes requested function
FrokResult frokAPIExecuteFunction(FrokAPI *instance, const char *functionName, const char *inJson, char **outJson);

// does nothing... yet :)
void frokAPIDeinit(FrokAPI *instance);
// delets instance
void frokAPIDealloc(void *instance);
#endif // C++

#endif // FROKAPI_H
