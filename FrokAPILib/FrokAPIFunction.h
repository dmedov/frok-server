#ifndef FROKAPIFUNCTION_H
#define FROKAPIFUNCTION_H

#define CUT_TIMEOUT            (600)
#define MAX_THREADS_AND_CASCADES_NUM        (1)

// include dependencies
#include <cv.h>
#include <highgui.h>
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

FrokResult Recognize(void *pContext);
FrokResult TrainUserModel(void *pContext);
FrokResult GetFacesFromPhoto(void *pContext);
FrokResult AddFaceFromPhoto(void *pContext);

#endif // FROKAPIFUNCTION_H
