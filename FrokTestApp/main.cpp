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
    if(!frokLibCommonInit(FROK_LIB_COMMON_DEFAULT_CONFIG_FILENAME))
    {
        TRACE_F("frokLibCommonInit");
        return -1;
    }

    FaceDetectorAbstract *detector = new FrokFaceDetector;
    FaceRecognizerAbstract *recognizer = new FaceRecognizerEigenfaces;

    FrokAPI fapi(commonContext->photoBasePath, commonContext->targetPhotosPath, detector, recognizer);

    fapi.AddAPIFunction("train", &FAPI_TrainUserModel);
    fapi.AddAPIFunction("recognize", &FAPI_Recognize);
    std::vector<std::string> functions;
    fapi.GetAvailableFunctions(functions);

    /*for(int i = 1; i < 18; i++)
    {
        std::stringstream str;
        str << i;
        FaceModelAbstract *model = new FaceModelEigenfaces(str.str());
        model->LoadUserModel((((std::string)DEFAULT_PHOTO_BASE_PATH).append(str.str()).append("/")).c_str());
        recognizer->AddFaceUserModel(str.str(), model);
    }*/

    std::string inJson= "{\"arrIds\": [\"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\",\
            \"8\", \"9\", \"10\", \"11\", \"12\", \"13\", \"14\", \"15\", \"16\", \"17\"]}";

    std::string outJson;
    fapi.ExecuteFunction("train", inJson, outJson);

    std::string inJsonRec1 = "{\"arrIds\": [\"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\",\
            \"8\", \"9\", \"10\", \"11\", \"12\", \"13\", \"14\", \"15\", \"16\", \"17\"],\
            \"photoName\": \"1.jpg\"}";
    std::string outJsonRec1;
    //fapi.ExecuteFunction("recognize", inJsonRec1, outJsonRec1);

    std::string inJsonRec2 = "{\"arrIds\": [\"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\",\
            \"8\", \"9\", \"10\", \"11\", \"12\", \"13\", \"14\", \"15\", \"16\", \"17\"],\
            \"photoName\": \"2.jpg\"}";
    std::string outJsonRec2;
    //fapi.ExecuteFunction("recognize", inJsonRec2, outJsonRec2);

    TRACE_N("1.jpg: %s\n", outJsonRec1.c_str());
    TRACE_N("2.jpg: %s\n", outJsonRec2.c_str());

    return 0;
}
