#ifndef FACERECOGNIZERABSTRACT_H
#define FACERECOGNIZERABSTRACT_H

// opencv dependencies
#include <cv.h>
#include <highgui.h>
#include <opencv2/contrib/contrib.hpp>

// include dependencies
#include "FaceModelAbstract.h"
#include "frokLibCommon.h"

class FaceRecognizerAbstract
{
protected:
    // All generated models
    std::map< std::string, FaceModelAbstract* >     models;
    // Models used for current recognition. Should be set in SetUserIdsVector function
    std::map< std::string, FaceModelAbstract* >     usedModels;
    cv::Mat                                         targetFace;
public:
    FaceRecognizerAbstract();
    virtual ~FaceRecognizerAbstract();
    virtual FrokResult SetTargetImage(cv::Mat &targetFace) = 0;

    virtual FrokResult SetUserIdsVector(std::vector<std::string> &usedUserIds) = 0;
    virtual FrokResult AddFaceUserModel(std::string userId, FaceModelAbstract *model) = 0;

    virtual FrokResult GetSimilarityOfFaceWithModels(std::map<std::string, double> &similarities) = 0;
};

#endif // FACERECOGNIZERABSTRACT_H
