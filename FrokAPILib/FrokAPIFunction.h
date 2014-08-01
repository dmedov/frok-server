#ifndef FROKAPIFUNCTION_H
#define FROKAPIFUNCTION_H

#define CUT_TIMEOUT            (600)
#define MAX_THREADS_AND_CASCADES_NUM        (1)

// include dependencies
#include <cv.h>
#include <highgui.h>

#include "FrokFaceDetector.h"
#include "FaceRecognizerEigenfaces.h"
#include "faceCommonLib.h"

typedef FrokResult (*APIFunction) (void *params);
typedef void (*Converter) (void* converterParams);
#pragma pack(push, 1)

typedef struct
{
    std::string jsonParameters;
    void *functionParameters;
}ConvertParams;

typedef struct FrokAPIFunction
{
public:
    // Pointer to function
    APIFunction         function;
    // Function parameters
    std::string         jsonParameters;
    // Function description
    std::string         functionDescription;
    // Function params description
    std::string         paramsDescription;
    // Estimated timeout in seconds
    unsigned long int   timeout;
    // Pointer to converter
    Converter ConvertJsonToFunctionParameters;
    // Pointer to converter
    Converter ConvertFunctionReturnToJson;
    FrokAPIFunction(APIFunction function, std::string jsonParameters, const char *functionDescription,
                    const char *paramsDescription, unsigned long int timeout, Converter json2funcP,
                    Converter funcR2json)
    {
        this->function = function;
        this->jsonParameters = jsonParameters;
        this->functionDescription = functionDescription;
        this->paramsDescription = paramsDescription;
        this->timeout = timeout;
        this->ConvertJsonToFunctionParameters = json2funcP;
        this->ConvertFunctionReturnToJson = funcR2json;
    }
}FrokAPIFunction;

#pragma pack(pop)

FrokResult TrainUserModel(std::vector<std::string> ids, const char *userBasePath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
FrokResult Recognize(std::vector< std::map<std::string, double> > &similarities, std::vector<std::string> ids, const char *userBasePath, std::string photoName, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
FrokResult GetFacesFromPhoto(void *pContext);
FrokResult AddFaceFromPhoto(void *pContext);

#endif // FROKAPIFUNCTION_H
