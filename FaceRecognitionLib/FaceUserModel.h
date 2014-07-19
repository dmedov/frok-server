#ifndef FACEUSERMODEL_H
#define FACEUSERMODEL_H

// include dependencies
#include "faceCommonLib.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>

// FaceRecognition defines
const char USER_MODEL_FILENAME[] = "eigenface.yml";

class FaceUserModel
{
public:
    FaceUserModel();
    FrokResult GenerateUserModel(const char *kappaFacesPath);
    FrokResult GenerateUserModel(std::vector<IplImage*> kappaFaces);
    FrokResult AddFaceToModel(IplImage *kappaFace);
    FrokResult AddFaceToModel(IplImage *kappaPhoto, cv::Rect faceCoords);
    FrokResult LoadUserModel(const char *userPath);
};

#endif // FACEUSERMODEL_H
