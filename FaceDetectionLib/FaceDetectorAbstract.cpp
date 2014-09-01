#include "FaceDetectorAbstarct.h"

#pragma GCC poison IplImage
#define MODULE_NAME     "FACE_DETECTOR_ABSTRACT"

FaceDetectorAbstract::FaceDetectorAbstract()
{
    TRACE_N("new FaceDetectorAbstract");
}

FaceDetectorAbstract::~FaceDetectorAbstract()
{
    TRACE_N("~FaceDetectorAbstract");
}
