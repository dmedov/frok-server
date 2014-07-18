#ifndef FROKAPI_H
#define FROKAPI_H

#include "../FaceCommonLib/faceCommonLib.h"

typedef FrokResult (*FrokAPIFunction) (void *params);

typedef struct FrokAPI
{
    // Pointer to function
    FrokAPIFunction     function;
    // Function description
    const char         *functionDescription;
    // template json with function's param fields with zeroed values
    std::string         jsonParams;
    // Function params description
    const char         *jsonParamsDescription;
    // Estimated timeout in seconds
    unsigned long int   timeout;
} FrokAPI;

FrokResult Recognize(void *param);
FrokResult  TrainUserModel(void *param);
FrokResult  GetFacesFromPhoto(void *param);
FrokResult  AddFaceFromPhoto(void *param);

extern FrokAPI FROK_API_FUNCTIONS[];
#endif // FROKAPI_H
