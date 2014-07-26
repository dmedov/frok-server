#ifndef FROKAPI_H
#define FROKAPI_H

#include <cv.h>
#include <highgui.h>
#include "faceCommonLib.h"

// FaceAgent defaults
const char DEFAULT_PHOTO_BASE_PATH [] = "/home/zda/faces/";
const char DEFAULT_TARGETS_FOLDER_PATH [] = "/home/zda/faces/";

typedef FrokResult (*APIFunction) (void *params);

class FrokAPI
{
    std::map<std::string, FrokAPIFunction> functions;
    FrokAPI();
    ~FrokAPI();
    void AddAPIFunction(std::string functionName, FrokAPIFunction function);
    FrokResult ExecuteFunction(std::string functionName);

    void GetAvailableFunctions(std::vector<std::string> &availableFunctions);
    void GetFrokAPIFunction(std::string functionName, FrokAPIFunction *function);
};

FrokResult Recognize(void *param);
FrokResult  TrainUserModel(void *param);
FrokResult  GetFacesFromPhoto(void *param);
FrokResult  AddFaceFromPhoto(void *param);

extern FrokAPI FROK_API_FUNCTIONS[];
#endif // FROKAPI_H
