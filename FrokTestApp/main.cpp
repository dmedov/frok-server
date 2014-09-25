#include <stdio.h>
#include "FrokAPI.h"
#include "frokLibCommon.h"

#define MODULE_NAME     "TEST"

void usage()
{
    TRACE_N("FaceDetectionApp <Local port number>");
    return;
}

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

int main(void)
{
    if(FROK_RESULT_SUCCESS != frokLibCommonInit(FROK_LIB_COMMON_DEFAULT_CONFIG_FILENAME))
    {
        TRACE_F("frokLibCommonInit");
        return -1;
    }

    TRACE_N("Create detector instance");
    void *detector = frokFaceDetectorAlloc();
    if(detector == NULL)
    {
        TRACE_F("Failed to create detector instance");
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("detector instance created");


    TRACE_N("Create recognizer instance");
    void *recognizer = frokFaceRecognizerEigenfacesAlloc();
    if(recognizer == NULL)
    {
        TRACE_F("Failed to create recognizer instance");
        frokFaceDetectorDealloc(detector);
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("recognizer instance created");

    TRACE_N("Init model with users from path %s", commonContext->photoBasePath);
    if(FALSE == frokFaceRecognizerEigenfacesInit(recognizer, commonContext->photoBasePath))
    {
        TRACE_F("Failed to init recognizer base. Continue");
    }
    else
    {
        TRACE_S("Recognize base inited");
    }


    void *fapi = frokAPIAlloc(commonContext->photoBasePath, commonContext->targetPhotosPath, detector, recognizer);
    frokAPIInit(fapi);

    std::string inJson = "{\"cmd\":\"recognize\", \"userIds\":[\"1\"], \"phName\":\"1.jpg\"}";

    char *outJson = NULL;

    frokAPIExecuteFunction(fapi, getFunctionFromJson(inJson.c_str()), inJson.c_str(), &outJson);

    TRACE_N("result: %s", outJson);

    return 0;
}
