#ifndef FROKAPI_H
#define FROKAPI_H

// include dependencies
#include "FrokAPIFunction.h"
#include "frokLibCommon.h"

#ifdef __cplusplus
typedef struct {
    double P;
    int ids;
    int x1;
    int x2;
    int y1;
    int y2;
} KeyElements;

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
#endif // C++

#ifdef __cplusplus
extern "C"{
#endif //c++
// Show result inage with detection and recognition results
void outputJsonOut(char* outJson);
// returns allocated in memory object
void *frokAPIAlloc(const char *photo_base_path, const char *targets_folder_path,
                   void *detector, void *recognizer);
// Adds all FrokAPIFunctions to instance
void frokAPIInit(void *instance);

// Verifies json and returns function name
char *getFunctionFromJson(const char *json);

// Executes requested function
FrokResult frokAPIExecuteFunction(void *instance, const char *functionName, const char *inJson, char **outJson);

// does nothing... yet :)
void frokAPIDeinit(void *instance);
// delets instance
void frokAPIDealloc(void *instance);
#ifdef __cplusplus
}
#endif //c++

#endif // FROKAPI_H
