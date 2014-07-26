#ifndef FROKAPIFUNCTION_H
#define FROKAPIFUNCTION_H

#include "string.h"
#include "json.h"

#define CUT_TIMEOUT            (600)
#define MAX_THREADS_AND_CASCADES_NUM        (1)


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

void recognizeFromModel(void *pContext);
void generateAndTrainBase(void *pContext);
void getFacesFromPhoto(void *pContext);
void saveFaceFromPhoto(void *pContext);

#endif // FROKAPIFUNCTION_H
