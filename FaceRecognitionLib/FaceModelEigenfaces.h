#ifndef FACEMODELEIGENFACES_H
#define FACEMODELEIGENFACES_H

// include dependencies
#include "FaceModelAbstract.h"

// FaceRecognition defines
const char USER_MODEL_FILENAME_EIGENFACES[] = "eigenface.yml";

class FaceModelEigenfaces : public FaceModelAbstract
{
protected:
    // [TBD] needed for Eigenfaces AddFaceToModel function
    std::vector<cv::Mat>        userGrayFaces;
    std::vector<int>            labels;
public:
    FaceModelEigenfaces();
    FaceModelEigenfaces(std::string userId);
    ~FaceModelEigenfaces();

    FrokResult GetPredictedFace(cv::Mat &targetFace, cv::Mat &predictedFace);
    FrokResult GenerateUserModel(const char *grayFacesPath);
    FrokResult GenerateUserModel(std::vector<cv::Mat> &grayFaces);
    FrokResult AddFaceToModel(cv::Mat &grayFace);
    FrokResult LoadUserModel(const char *userPath);
    FrokResult SaveUserModel(const char *userPath);
};

#endif // FACEMODELEIGENFACES_H
