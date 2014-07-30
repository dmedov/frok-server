#include "FaceRecognizerAbstract.h"
#include "FaceModelEigenfaces.h"
#include "FaceModelAbstract.h"
#include "unistd.h"
#define MODULE_NAME     "FACE_RECOGNIZER_ABSTRACT"

FaceRecognizerAbstract::FaceRecognizerAbstract()
{
    TRACE("new FaceRecognizerAbstract");
}
FaceRecognizerAbstract::~FaceRecognizerAbstract()
{
    TRACE("~FaceRecognizerAbstract");
}
