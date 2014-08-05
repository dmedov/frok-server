#ifndef FROKAPI_H
#define FROKAPI_H

// include dependencies
#include "FrokAPIFunction.h"
#include "faceCommonLib.h"

// FaceAgent defaults
const char DEFAULT_PHOTO_BASE_PATH [] = "/home/zda/faces/";
const char DEFAULT_TARGETS_FOLDER_PATH [] = "/home/zda/faces/";

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
    FrokAPI(FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
    ~FrokAPI();
    void AddAPIFunction(std::string functionName, FrokAPIFunction *function);
    FrokResult ExecuteFunction(std::string functionName, std::string inJson, std::string &outJson);

    void GetAvailableFunctions(std::vector<std::string> &availableFunctions);
};

extern FrokAPI FROK_API_FUNCTIONS[];
#endif // FROKAPI_H
