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

    FaceDetectorAbstract *detector = new FrokFaceDetector;
    FaceRecognizerAbstract *recognizer = new FaceRecognizerEigenfaces;

    void *fapi = frokAPIAlloc(commonContext->photoBasePath, commonContext->targetPhotosPath, detector, recognizer);
    frokAPIInit(fapi);

    std::string inJson = "{\"cmd\":\"train\", \"arrUserIds\":[\"1\"]}";

    char *outJson = NULL;

    frokAPIExecuteFunction(fapi, getFunctionFromJson(inJson.c_str()), inJson.c_str(), &outJson);

    TRACE_N("result: %s", outJson);

    return 0;
}
