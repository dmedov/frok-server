#ifndef FrokFaceRecognizer_H
#define FrokFaceRecognizer_H

// include dependencies
#include "FaceRecognizerAbstract.h"
#include "FaceUserModel.h"
#include "faceCommonLib.h"

typedef std::pair<std::string, FaceUserModel> userIdAndModel;
typedef std::pair<std::string, double> userIdAndSimilarity;
class FrokFaceRecognizer : public FaceRecognizerAbstract
{
private:
    unsigned maxHammingDistance;
public:
    FrokFaceRecognizer();
    ~FrokFaceRecognizer();
    FrokResult SetTargetImage(cv::Mat &targetFace);
    FrokResult SetUserIdsVector(std::vector<std::string> &usedUserIds);
    FrokResult AddFrokUserModel(std::string userId, FaceUserModel &model);
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

#endif // FrokFaceRecognizer_H
