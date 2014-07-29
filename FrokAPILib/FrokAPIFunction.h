#ifndef FROKAPIFUNCTION_H
#define FROKAPIFUNCTION_H

#define CUT_TIMEOUT            (600)
#define MAX_THREADS_AND_CASCADES_NUM        (1)

// include dependencies
#include <cv.h>
#include <highgui.h>

#include "FrokFaceDetector.h"
#include "FrokFaceRecognizer.h"
#include "faceCommonLib.h"

typedef FrokResult (*APIFunction) (void *params);

#pragma pack(push, 1)

typedef struct FrokAPIFunction
{
public:
    // Pointer to function
    APIFunction function;
    // Function description
    const char         *functionDescription;
    // template json with function's param fields with zeroed values
    std::string         jsonParams;
    // Function params description
    const char         *jsonParamsDescription;
    // Estimated timeout in seconds
    unsigned long int   timeout;
}FrokAPIFunction;

#pragma pack(pop)

FrokResult TrainUserModel(std::vector<std::string> ids, const char *userBasePath, FaceDetectorAbstract *detector, FaceRecognizerAbstract *recognizer);
FrokResult Recognize(std::vector< std::map<std::string, double> > &similarities, std::vector<std::string> ids, const char *userBasePath, std::string photoName, const char *targetPhotosPath, FaceDetectorAbstract *detector, FaceDetectorAbstract *recognizer);
FrokResult GetFacesFromPhoto(void *pContext);
FrokResult AddFaceFromPhoto(void *pContext);

#endif // FROKAPIFUNCTION_H
