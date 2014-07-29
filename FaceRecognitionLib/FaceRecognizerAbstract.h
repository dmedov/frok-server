#ifndef FACERECOGNIZERABSTRACT_H
#define FACERECOGNIZERABSTRACT_H

// include dependencies
#include "faceCommonLib.h"
#include "FaceUserModel.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>
#include <opencv2/contrib/contrib.hpp>

class FaceRecognizerAbstract
{
protected:
    std::map<std::string, FaceUserModel>    models;
    std::map<std::string, FaceUserModel>    usedModels;
    cv::Mat                                 targetFace;
public:
    FaceRecognizerAbstract();
    virtual ~FaceRecognizerAbstract();
    virtual FrokResult SetTargetImage(cv::Mat &targetFace) = 0;

    virtual FrokResult SetUserIdsVector(std::vector<std::string> &usedUserIds) = 0;
    virtual FrokResult AddFrokUserModel(std::string userId, FaceUserModel &model) = 0;

    virtual FrokResult GetSimilarityOfFaceWithModels(std::map<std::string, double> &similarities) = 0;
};

#endif // FACERECOGNIZERABSTRACT_H
