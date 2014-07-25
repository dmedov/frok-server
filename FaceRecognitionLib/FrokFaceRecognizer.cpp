#include "FrokFaceRecognizer.h"

#define MODULE_NAME     "FACE_RECOGNIZER"

FrokFaceRecognizer::FrokFaceRecognizer()
{
    // Set defaults
    maxHammingDistance = 18;
    TRACE("new FrokFaceRecognizer");
}

FrokFaceRecognizer::~FrokFaceRecognizer()
{
    TRACE("~FrokFaceRecognizer");
}

FrokResult FrokFaceRecognizer::AddFrokUserModel(std::string userId, FaceUserModel &model)
{
    TRACE_T("started");
    models[userId] = model;
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

double FrokFaceRecognizer::GetSimilarity_FirstMethod(const cv::Mat firstImage, const cv::Mat secondImage)
{
    TRACE_T("started");
    cv::Mat diffMat = firstImage - secondImage;

    double err = 0;
    for(int row = 0; row < diffMat.rows; row++)
    {
        unsigned char* points  = diffMat.ptr<unsigned char>(row);
        for (int col = 0; col < diffMat.cols; col++)
        {
            unsigned char pointValue = *points++;
            if(pointValue > 40)
            {
                err += pointValue;
            }
        }
    }
    TRACE_T("Calculated error = %lf", err);
    if(err < 0) err = -err;
    err /= (255 * diffMat.rows * diffMat.cols);
    TRACE_T("Normalized error = %lf", err);
    TRACE_T("finished");
    return 1 - err;
}
double FrokFaceRecognizer::GetSimilarity_SecondMethod(const cv::Mat firstImage, const cv::Mat secondImage)
{
    TRACE_T("started");
    cv::Mat temporary1, temporary2;
    try
    {
        cv::cornerMinEigenVal(firstImage, temporary1, 20, 7);
        cv::cornerMinEigenVal(secondImage, temporary2, 20, 7);
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to process cornerMinEigenVal function");
        return -1;
    }
    cv::Mat diffMat = temporary1 - temporary2;

    double err = 0;
    for(int row = 0; row < diffMat.rows; row++)
    {
        unsigned char* points  = diffMat.ptr<unsigned char>(row);
        for (int col = 0; col < diffMat.cols; col++)
        {
            err += *points++;
        }
    }

    TRACE_T("Calculated error = %lf", err);
    if(err < 0) err = -err;
    err /= (255 * diffMat.rows * diffMat.cols);
    TRACE_T("Normalized error = %lf", err);
    TRACE_T("finished");
    return 1 - err;
}

double FrokFaceRecognizer::GetSimilarity_ThirdMethod(const cv::Mat &firstImage, const cv::Mat &secondImage)
{
    TRACE_T("started");
    // Calculate the L2 relative error between the 2 images.
    double err = cv::norm(firstImage, secondImage, CV_L2);
    // Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
    TRACE_T("Calculated error = %lf", err);
    err /= (firstImage.rows * firstImage.cols);
    TRACE_T("Normalized error = %lf", err);
    TRACE_T("finished");
    return 1 - err;
}

FrokResult FrokFaceRecognizer::SetUserIdsVector(std::vector<std::string> &usedUserIds)
{
    TRACE_T("started");
    if(models.empty())
    {
        TRACE_F_T("No models loaded. Add some user models first");
        return FROK_RESULT_NO_MODELS;
    }
    usedModels.clear();
    for(std::vector<std::string>::const_iterator it = usedUserIds.begin(); it != usedUserIds.end(); ++it)
    {
        if(models.end() != models.find(*it))
        {
            std::string userId = *it;
            TRACE_S_T("User %s model found", userId.c_str());
            usedModels[userId] = models[userId];
        }
    }
    if(usedModels.empty())
    {
        TRACE_F_T("no models found");
        return FROK_RESULT_NO_MODELS;
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FrokFaceRecognizer::GetSimilarityOfFaceWithModels(cv::Mat &targetFace, std::map<std::string, double> &similarities)
{
    TRACE_T("started");
    if(usedModels.empty())
    {
        TRACE_F_T("No models specified for this call. Call SetUserIdsVector function first");
        return FROK_RESULT_NO_MODELS;
    }

    FrokResult res;
    for(std::map<std::string, FaceUserModel>::const_iterator it = usedModels.begin(); it != usedModels.end(); ++it)
    {
        FaceUserModel model = ((userIdAndModel)*it).second;
        std::string userId = ((userIdAndModel)*it).first;
        cv::Mat predictedFace;
        if(FROK_RESULT_SUCCESS != (res = model.GetPredictedFace(targetFace, predictedFace)))
        {
            TRACE_F_T("GetPredictedFace failed on result %x for user %s", res, userId.c_str());
            continue;
        }

        __int64_t hammingDistance = calcHammingDistance(calcImageHash(targetFace), calcImageHash(predictedFace));

        if (hammingDistance <= maxHammingDistance)
        {
            double prob1 = GetSimilarity_FirstMethod(targetFace, predictedFace);
            double prob2 = GetSimilarity_SecondMethod(targetFace, predictedFace);
            double prob3 = GetSimilarity_ThirdMethod(targetFace, predictedFace);

            double geometricMean = pow(prob1*prob2*prob3, 1. / 3);
            double arithmeticMean = (prob1 + prob2 + prob3) / 3;

            TRACE_S_T("geometric mean probability = %lf", geometricMean);
            TRACE_S_T("arithmetic mean probability = %lf", arithmeticMean);
            similarities[userId] = geometricMean;
        }
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

// [TBD] Get any prove of this code frok Nikita!
__int64_t FrokFaceRecognizer::calcImageHash(cv::Mat &image)
{
    TRACE_T("started");

    cv::Mat imgWithThreshold;
    cv::Scalar average = cv::mean(image);
    cv::threshold(image, imgWithThreshold, average.val[0], 255, CV_THRESH_BINARY);

    __int64_t hash = 0;
    int i = 0;

    for (int y = 0; y < imgWithThreshold.rows; y++)
    {
        uchar* colArray = (uchar*)(imgWithThreshold.data + y * imgWithThreshold.step);
        for (int x = 0; x < imgWithThreshold.cols; x++)
        {
            if (colArray[x])
            {
                hash |= (__int64_t)1 << i;
            }
            i++;
        }
    }

    TRACE_T("finished");

    return hash;
}

// [TBD] There is possibly easier way to count it
__int64_t FrokFaceRecognizer::calcHammingDistance(__int64_t firstHash, __int64_t secondHash)
{
    TRACE_T("started");
    __int64_t dist = 0, val = firstHash ^ secondHash;

    // Count the number of set bits
    while (val)
    {
        ++dist;
        val &= val - 1;
    }

    TRACE_T("finished");
    return dist;
}

