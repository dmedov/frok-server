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

    FaceRecognizerAbstract *recognizer = new FaceRecognizerEigenfaces;
    FaceDetectorAbstract *detector = new FrokFaceDetector;

    ConvertParams params;
    ConvertParams outParams;
    //params.jsonParameters = "{\"arrIds\": [\"3\", \"4\"]}";
    params.jsonParameters = "{\"arrIds\": [\"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\",\
            \"8\", \"9\", \"10\", \"11\", \"12\", \"13\", \"14\", \"15\", \"16\", \"17\"]}";

    FAPI_TrainUserModel.ConvertJsonToFunctionParameters(&params);
    FAPI_TrainUserModel.function(params.functionParameters, &outParams.functionParameters, DEFAULT_PHOTO_BASE_PATH, DEFAULT_TARGETS_FOLDER_PATH, detector, recognizer);
    FAPI_TrainUserModel.ConvertFunctionReturnToJson(&outParams);

    ConvertParams params1;
    ConvertParams outParams1;
    params1.jsonParameters = "{\"arrIds\": [\"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\",\
            \"8\", \"9\", \"10\", \"11\", \"12\", \"13\", \"14\", \"15\", \"16\", \"17\"],\
            \"photoName\": \"1.jpg\"}";
    FAPI_Recognize.ConvertJsonToFunctionParameters(&params1);
    FAPI_Recognize.function(params1.functionParameters, &outParams1.functionParameters, DEFAULT_PHOTO_BASE_PATH, DEFAULT_TARGETS_FOLDER_PATH, detector, recognizer);
    FAPI_Recognize.ConvertFunctionReturnToJson(&outParams1);

    ConvertParams params2;
    ConvertParams outParams2;
    params2.jsonParameters = "{\"arrIds\": [\"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\",\
            \"8\", \"9\", \"10\", \"11\", \"12\", \"13\", \"14\", \"15\", \"16\", \"17\"],\
            \"photoName\": \"2.jpg\"}";
    FAPI_Recognize.ConvertJsonToFunctionParameters(&params2);
    FAPI_Recognize.function(params2.functionParameters, &outParams2.functionParameters, DEFAULT_PHOTO_BASE_PATH, DEFAULT_TARGETS_FOLDER_PATH, detector, recognizer);
    FAPI_Recognize.ConvertFunctionReturnToJson(&outParams2);

    printf("1.jpg: %s\n", outParams1.jsonParameters.c_str());
    printf("2.jpg: %s\n", outParams2.jsonParameters.c_str());

    return 0;
}
