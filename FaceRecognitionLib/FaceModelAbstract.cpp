#include "FaceModelAbstract.h"

#define MODULE_NAME     "FACE_MODEL_ABSTRACT"

FaceModelAbstract::FaceModelAbstract()
{
    TRACE_F("This constructor must not be called");
}

FaceModelAbstract::FaceModelAbstract(std::string userId)
{
    TRACE("new FaceModelAbstract");
    this->userId = userId;
}

FaceModelAbstract::~FaceModelAbstract()
{
    TRACE("~FaceModelAbstract");
}
