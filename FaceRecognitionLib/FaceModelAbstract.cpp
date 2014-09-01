#include "FaceModelAbstract.h"

#pragma GCC poison IplImage
#define MODULE_NAME     "FACE_MODEL_ABSTRACT"

FaceModelAbstract::FaceModelAbstract()
{
    TRACE_F("This constructor must not be called");
}

FaceModelAbstract::FaceModelAbstract(std::string userId)
{
    TRACE_N("new FaceModelAbstract");
    this->userId = userId;
}

FaceModelAbstract::~FaceModelAbstract()
{
    TRACE_N("~FaceModelAbstract");
}
