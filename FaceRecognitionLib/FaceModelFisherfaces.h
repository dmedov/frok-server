#ifndef FACEMODELFISHERFACES_H
#define FACEMODELFISHERFACES_H

// include dependencies
#include "FaceModelAbstract.h"

// FaceRecognition defines
const char USER_MODEL_FILENAME_FISHERFACES[] = "eigenface.yml";

class FaceModelFisherfaces : public FaceModelAbstract
{
protected:
    // [TBD] needed for Fisherface AddFaceToModel function
    std::vector<cv::Mat>        userGrayFaces;
    std::vector<int>            labels;
public:
    FaceModelFisherfaces();
    FaceModelFisherfaces(FaceModelAbstract *m);
    FaceModelFisherfaces(std::string userId);
    ~FaceModelFisherfaces();

    FrokResult GetPredictedFace(cv::Mat &targetFace, cv::Mat &predictedFace);
    FrokResult GenerateUserModel(const char *grayFacesPath);
    FrokResult GenerateUserModel(std::vector<cv::Mat> &grayFaces);
    FrokResult AddFaceToModel(cv::Mat &grayFace);
    FrokResult LoadUserModel(const char *userPath);
    FrokResult SaveUserModel(const char *userPath);
};

#endif // FACEMODELFISHERFACES_H
