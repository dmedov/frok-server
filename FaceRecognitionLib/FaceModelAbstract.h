#ifndef FACEMODELABSTRACT_H
#define FACEMODELABSTRACT_H

// opencv dependencies
#include <cv.h>
#include <highgui.h>
#include <opencv2/contrib/contrib.hpp>

// include dependencies
#include "frokLibCommon.h"

class FaceModelAbstract
{
protected:
    // FaceRecognizer model for this user
    cv::Ptr<cv::FaceRecognizer> model;
    // This user's user id
    std::string                 userId;
public:
    // Don't use this constructor
    FaceModelAbstract();
    // Use this constructor
    FaceModelAbstract(std::string userId);
    virtual ~FaceModelAbstract();

    virtual FrokResult GetPredictedFace(cv::Mat &targetFace, cv::Mat &predictedFace) = 0;
    virtual FrokResult GenerateUserModel(const char *grayFacesPath) = 0;
    virtual FrokResult GenerateUserModel(std::vector<cv::Mat> &grayFaces) = 0;
    virtual FrokResult AddFaceToModel(cv::Mat &grayFace) = 0;
    virtual FrokResult LoadUserModel(const char *userPath) = 0;
    virtual FrokResult SaveUserModel(const char *userPath) = 0;
};

#endif // FACEMODELABSTRACT_H
