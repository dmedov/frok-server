#ifndef FROKAPIFUNCTION_H
#define FROKAPIFUNCTION_H

#define CUT_TIMEOUT            (600)
#define MAX_THREADS_AND_CASCADES_NUM        (1)

#include "FrokFaceDetector.h"
#include "FaceRecognizerEigenfaces.h"
#include "frokLibCommon.h"

#ifdef __cplusplus

// include dependencies
#include <cv.h>
#include <highgui.h>

#include "json.h"

#pragma pack(push, 1)

typedef struct
{
    std::string jsonParameters;
    void *functionParameters;
}ConvertParams;

typedef FrokResult (*APIFunction) (void *inParams, void **outParams, const char *userBasePath, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
typedef bool (*Converter) (ConvertParams *converterParams);

typedef struct FrokAPIFunction
{
public:
    // Pointer to function
    APIFunction         function;
    // Function parameters
    std::string         inJsonParameters;
    std::string         outJsonParameters;
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
    FrokAPIFunction(const FrokAPIFunction &copy)
    {
        this->function = copy.function;
        this->inJsonParameters = copy.inJsonParameters;
        this->outJsonParameters = copy.outJsonParameters;
        this->functionDescription = copy.functionDescription;
        this->timeout = copy.timeout;
        this->ConvertFunctionReturnToJson = copy.ConvertFunctionReturnToJson;
        this->ConvertJsonToFunctionParameters = copy.ConvertJsonToFunctionParameters;
        this->paramsDescription = copy.paramsDescription;
    }
    FrokAPIFunction(APIFunction function, std::vector<std::string> inJsonParameters, std::vector<std::string> outJsonParameters,
                    const char *functionDescription, const char *paramsDescription, unsigned long int timeout,
                    Converter json2funcP, Converter funcR2json)
    {
        this->function = function;
        json::Object obj;
        for(std::vector<std::string>::iterator it = inJsonParameters.begin(); it != inJsonParameters.end(); ++it)
        {
            obj[(std::string)*it] = "";
        }
        this->inJsonParameters = json::Serialize(obj);
        obj.Clear();
        for(std::vector<std::string>::iterator it = outJsonParameters.begin(); it != outJsonParameters.end(); ++it)
        {
            obj[(std::string)*it] = "";
        }
        this->outJsonParameters = json::Serialize(obj);
        this->functionDescription = functionDescription;
        this->paramsDescription = paramsDescription;
        this->timeout = timeout;
        this->ConvertJsonToFunctionParameters = json2funcP;
        this->ConvertFunctionReturnToJson = funcR2json;
    }
}FrokAPIFunction;

#pragma pack(pop)

extern FrokAPIFunction FAPI_TrainUserModel;
extern FrokAPIFunction FAPI_Recognize;
extern FrokAPIFunction FAPI_GetFacesFromPhoto;
extern FrokAPIFunction FAPI_AddFaceFromPhotoToModel;

#endif // C++

#endif // FROKAPIFUNCTION_H
