#ifndef FACERECOGNIZEREIGENFACES_H
#define FACERECOGNIZEREIGENFACES_H

// include dependencies
#include "FaceRecognizerAbstract.h"
#include "FaceModelEigenfaces.h"
#include "faceCommonLib.h"

typedef std::pair< std::string, FaceModelAbstract* > userIdAndModel;
typedef std::pair< std::string, double > userIdAndSimilarity;
class FaceRecognizerEigenfaces : public FaceRecognizerAbstract
{
private:
    unsigned maxHammingDistance;
public:
    FaceRecognizerEigenfaces();
    ~FaceRecognizerEigenfaces();
    FrokResult SetTargetImage(cv::Mat &targetFace);
    FrokResult SetUserIdsVector(std::vector<std::string> &usedUserIds);
    FrokResult AddFaceUserModel(std::string userId, FaceModelAbstract *model);
    FrokResult GetSimilarityOfFaceWithModels(std::map<std::string, double> &similarities);
private:
    double GetSimilarity_FirstMethod(const cv::Mat firstImage, const cv::Mat secondImage);
    double GetSimilarity_SecondMethod(const cv::Mat firstImage, const cv::Mat secondImage);
    double GetSimilarity_ThirdMethod(const cv::Mat &firstImage, const cv::Mat &secondImage);
    double GetSimilarity_ChiSquare(const cv::Mat &firstImage, const cv::Mat &secondImage);

    FrokResult GetImageHistogram(const cv::Mat &image, cv::MatND &histogram);
    __int64_t calcImageHash(cv::Mat &image);
    __int64_t calcHammingDistance(__int64_t firstHash, __int64_t secondHash);
};

#endif // FACERECOGNIZEREIGENFACES_H
