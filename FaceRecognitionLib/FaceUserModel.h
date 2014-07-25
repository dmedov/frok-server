#ifndef FACEUSERMODEL_H
#define FACEUSERMODEL_H

// include dependencies
#include "faceCommonLib.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>
#include <opencv2/contrib/contrib.hpp>

// FaceRecognition defines
const char USER_MODEL_FILENAME_EIGENFACES[] = "eigenface.yml";
const char USER_MODEL_FILENAME_FISHER[] = "fisher.yml";

typedef enum
{
    RECOGNIZER_EIGENFACES   = 0x00,
    RECOGNIZER_FISHER       = 0x01,
}EnumFaceRecognizer;

class FaceUserModel
{
private:
    EnumFaceRecognizer          modelType;
    cv::Ptr<cv::FaceRecognizer> model;
    std::string                 userId;
    // [TBD] needed for Eigenfaces AddFaceToModel function
    std::vector<cv::Mat>        userKappaFaces;
    std::vector<int>    labels;
public:
    FaceUserModel();
    FaceUserModel(std::string userId, EnumFaceRecognizer recognizer);
    ~FaceUserModel();
    FrokResult GetPredictedFace(cv::Mat &targetFace, cv::Mat &predictedFace);
    FrokResult GenerateUserModel(const char *kappaFacesPath);
    FrokResult GenerateUserModel(std::vector<cv::Mat> &kappaFaces);
    FrokResult AddFaceToModel(cv::Mat &kappaFace);
    FrokResult LoadUserModel(const char *userPath);
    FrokResult SaveUserModel(const char *userPath);
};

#endif // FACEUSERMODEL_H
