#ifndef FACEUSERMODEL_H
#define FACEUSERMODEL_H

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>

// FaceRecognition defines
const char USER_MODEL_FILENAME[] = "eigenface.yml";

class FaceUserModel
{
public:
    FaceUserModel();
    GenerateUserModel(const char *kappaFacesPath);
    GenerateUserModel(std::vector<IplImage*> kappaFaces);
    AddFaceToModel(IplImage *kappaFace);
    AddFaceToModel(IplImage *kappaPhoto, cv::Rect faceCoords);
    LoadUserModel(const char *userPath);
};

#endif // FACEUSERMODEL_H
