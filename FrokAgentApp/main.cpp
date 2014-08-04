#include <stdio.h>
#include "FrokAgent.h"

void usage()
{
    printf("FaceDetectionApp <Local port number>");
    return;
}

#define MODULE_NAME ""

int main(void)
{
    if(!InitFaceCommonLib())
    {
        TRACE_F("InitFaceCommonLib");
        return -1;
    }

    std::map <std::string, FrokAPIFunction*> functions;
    functions["train"] = &FAPI_TrainUserModel;
    functions["recognize"] = &FAPI_Recognize;

    FrokAgent agent(functions);
    agent.StartFrokAgent();
    getchar();
    agent.StopFrokAgent();

    DeinitFaceCommonLib();

    return 0;
}
