#include "FaceRecognizerEigenfaces.h"

#pragma GCC poison IplImage

#define MODULE_NAME     "FACE_RECOGNIZER_EIGENFACES"

extern "C" {

void *frokFaceRecognizerEigenfacesAlloc()
{
    FaceRecognizerEigenfaces *instance = NULL;
    try
    {
        instance = new FaceRecognizerEigenfaces;
    }
    catch(...)
    {
        TRACE_F("FaceRecognizerEigenfaces constructor failed");
        return NULL;
    }
    return instance;
}
void frokFaceRecognizerEigenfacesDealloc(void *instance)
{
    if(instance != NULL)
    {
        delete (FaceModelEigenfaces*)instance;
        instance = NULL;
    }
}

BOOL frokFaceRecognizerEigenfacesInit(void *instance, const char *photoBasePath)
{
    char **users = NULL;
    unsigned usersNum = 0;
    FrokResult res;

    if(instance == NULL || photoBasePath == NULL)
    {
        TRACE_F("Invalid parameter: instance = %p, photoBasePath = %p", instance, photoBasePath);
        return FALSE;
    }

    if(FALSE == getSubdirsFromDir(photoBasePath, &users, &usersNum))
    {
        TRACE_F("getSubdirsFromDir failed");
        return FALSE;
    }

    for(int i = 0; i < usersNum; i++)
    {
        std::string user = users[i];
        std::string userPath = photoBasePath;
        userPath.append(user).append("/");
        FaceModelAbstract *model;
        try
        {
            model = new FaceModelFisherfaces(user);
            if(FROK_RESULT_SUCCESS != (res = model->LoadUserModel(userPath.c_str())))
            {
                TRACE_F("LoadUserModel for user %s failed on error %s, continue...", user.c_str(), FrokResultToString(res));
                continue;
            }
        }
        catch(...)
        {
            TRACE_F("Failed to create model. Continue...");
            continue;
        }

        if(FROK_RESULT_SUCCESS != (res = ((FaceRecognizerEigenfaces*) instance)->AddFaceUserModel(user, model)))
        {
            TRACE_F("Failed to add user model for user %s on error %s, continue...", user.c_str(), FrokResultToString(res));
            continue;
        }
    }
    return TRUE;
}

}

FaceRecognizerEigenfaces::FaceRecognizerEigenfaces()
{
    // Set defaults
    maxHammingDistance = 18;
    TRACE_N("new FaceRecognizerEigenfaces");
}

FaceRecognizerEigenfaces::~FaceRecognizerEigenfaces()
{
    TRACE_N("~FaceRecognizerEigenfaces");
}

FrokResult FaceRecognizerEigenfaces::AddFaceUserModel(std::string userId, FaceModelAbstract *model)
{
    TRACE_T("started");
    models[userId] = model;
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

double FaceRecognizerEigenfaces::GetSimilarity_FirstMethod(const cv::Mat firstImage, const cv::Mat secondImage)
{
    TRACE_T("started");

    cv::Mat firstImageCopy;
    cv::Mat secondImageCopy;
    firstImage.copyTo(firstImageCopy);
    secondImage.copyTo(secondImageCopy);
    cv::erode(firstImageCopy, firstImageCopy, firstImageCopy);
    cv::erode(secondImageCopy, secondImageCopy, secondImageCopy);
    cv::Mat diffMat = firstImageCopy - secondImageCopy;

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
    //TRACE_T("Calculated error = %lf", err);
    if(err < 0) err = -err;
    err /= (255 * diffMat.rows * diffMat.cols);
    TRACE_R("GetSimilarity_FirstMethod result = %lf", (1 - err));
    TRACE_T("finished");
    return 1 - err;
}
double FaceRecognizerEigenfaces::GetSimilarity_SecondMethod(const cv::Mat firstImage, const cv::Mat secondImage)
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

    //TRACE_T("Calculated error = %lf", err);
    if(err < 0) err = -err;
    err /= (255 * diffMat.rows * diffMat.cols);
    TRACE_R("GetSimilarity_SecondMethod result = %lf", (1 - err));
    TRACE_T("finished");
    return 1 - err;
}

double FaceRecognizerEigenfaces::GetSimilarity_ThirdMethod(const cv::Mat &firstImage, const cv::Mat &secondImage)
{
    TRACE_T("started");
    // Calculate the L2 relative error between the 2 images.
    double err = cv::norm(firstImage, secondImage, CV_L2);
    // Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
    //TRACE_T("Calculated error = %lf", err);
    err /= (firstImage.rows * firstImage.cols);
    TRACE_R("GetSimilarity_ThirdMethod result = %lf", (1 - err));
    TRACE_T("finished");
    return 1 - err;
}

double FaceRecognizerEigenfaces::GetSimilarity_ChiSquare(const cv::Mat &firstImage, const cv::Mat &secondImage)
{
    TRACE_T("started");
    FrokResult res;

    cv::MatND firstImageHist;
    cv::MatND secondImageHist;

    double resPercent = 0;

    TRACE_T("Getting first image's histogram");

    if(FROK_RESULT_SUCCESS != (res = GetImageHistogram(firstImage, firstImageHist)))
    {
        TRACE_F_T("GetImageHistogram failed on result %s", FrokResultToString(res));
        return -1;
    }

    TRACE_T("Getting second image's histogram");

    if(FROK_RESULT_SUCCESS != (res = GetImageHistogram(secondImage, secondImageHist)))
    {
        TRACE_F_T("GetImageHistogram failed on result %s", FrokResultToString(res));
        return -1;
    }

    TRACE_T("Comparing histograms");
    double chiSquare = 0;
    try
    {
        chiSquare = cv::compareHist(firstImageHist, secondImageHist, CV_COMP_CHISQR);
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to compareHist");
        return -1;
    }

    resPercent = GetPercantByChiSqruare(2, chiSquare);
    TRACE_R("GetSimilarity_ChiSquare result = %lf", resPercent);
    TRACE_T("finished");
    return resPercent;
}

FrokResult FaceRecognizerEigenfaces::SetUserIdsVector(std::vector<std::string> &usedUserIds)
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

FrokResult FaceRecognizerEigenfaces::SetTargetImage(cv::Mat &targetFace)
{
    TRACE_T("started");
    try
    {
        targetFace.copyTo(this->targetFace);
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to copy tagetFace");
        return FROK_RESULT_OPENCV_ERROR;
    }

    TRACE_S("Target photo size: %dx%d", this->targetFace.cols, this->targetFace.rows);
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceRecognizerEigenfaces::GetSimilarityOfFaceWithModels(std::map<std::string, double> &similarities)
{
    TRACE_T("started");
    if(usedModels.empty())
    {
        TRACE_F_T("No models specified for this call. Call SetUserIdsVector function first");
        return FROK_RESULT_NO_MODELS;
    }

    FrokResult res;
    for(std::map< std::string, FaceModelAbstract* >::const_iterator it = usedModels.begin(); it != usedModels.end(); ++it)
    {
        FaceModelAbstract *model = ((userIdAndModel)*it).second;
        std::string userId = ((userIdAndModel)*it).first;
        cv::Mat predictedFace;

        TRACE_T("Getting similarity for user %s", userId.c_str());
        if(FROK_RESULT_SUCCESS != (res = model->GetPredictedFace(targetFace, predictedFace)))
        {
            TRACE_F_T("GetPredictedFace failed on result %s for user %s", FrokResultToString(res), userId.c_str());
            continue;
        }

//        int n = 3; // Количество точек
//        CvPoint *pts = new CvPoint[sizeof(CvPoint)*n]; // Массив точек
//        pts[0] = cvPoint(10,10);
//        pts[1] = cvPoint(50,50);
//        pts[2] = cvPoint(50,10);
//        cvFillPoly(targetFace, &pts, &n, 1, CV_RGB(0,255,0));


//        cv::imshow("targetFace", targetFace);
//        cv::waitKey(0);


        __int64_t hammingDistance = calcHammingDistance(calcImageHash(targetFace), calcImageHash(predictedFace));

        if (hammingDistance <= maxHammingDistance)
        {

            double prob1_o = GetSimilarity_FirstMethod_old(targetFace, predictedFace);
            double prob2_o = GetSimilarity_SecondMethod_old(targetFace, predictedFace);
            double prob3_o = GetSimilarity_ThirdMethod_old(targetFace, predictedFace);

            double prob_res1 = pow(prob1_o*prob2_o*prob3_o, 1. / 3);

            double prob_max = std::max(std::max(prob3_o, prob2_o), prob1_o);
            double prob_min = std::min(std::min(prob3_o, prob2_o), prob1_o);

            double prob = (prob_res1 - (prob_max - prob_min))/1.5;

            TRACE_S_T("User %s results:", userId.c_str());
            similarities[userId] = fabs(prob);
        }
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

double FaceRecognizerEigenfaces::GetSimilarity_FirstMethod_old(const cv::Mat &firstImage, const cv::Mat &secondImage)
{
    cv::Mat blr_img;
    firstImage.copyTo(blr_img);

    cv::Mat blr_rec;
    secondImage.copyTo(blr_rec);

    cv::GaussianBlur(blr_img, blr_img, cv::Size(7,7), 0, 0);
    cv::GaussianBlur(blr_rec, blr_rec, cv::Size(7,7), 0, 0);

    cv::Mat dif = abs(cv::Mat(blr_img) - cv::Mat(blr_rec));

    int koef = 0;
    double err = 0;
    for (int y(0); y < dif.rows; ++y){
        for (int x(0); x < dif.cols; ++x){
            int d = dif.at<unsigned char>(y, x);
            if (d >= 40)
                koef += d;
        }
    }

    err = (double)koef / ((double)dif.cols * (double)dif.rows * 40);

    double prob = (1 - err);

    if (prob > 1) prob = 0.99;
    if (prob < 0) prob = 0.01;

    return prob;
}

// Compare two images by getting the L2 error (square-root of sum of squared error).
double FaceRecognizerEigenfaces::GetSimilarity_SecondMethod_old(const cv::Mat &firstImage, const cv::Mat &secondImage)
{
    if ((firstImage.rows > 0) && (firstImage.rows == secondImage.rows ) &&
        (firstImage.cols > 0) && (firstImage.cols == secondImage.cols)) {
        // Calculate the L2 relative error between the 2 images.
        double errorL2 = norm(firstImage, secondImage, CV_L2);
        // Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
        double prob = 1 - (errorL2 / (double)(firstImage.rows * firstImage.cols));
        if (prob > 1) prob = 0.99;
        if (prob < 0) prob = 0.01;
        return prob;
    }
    else return 0.01;

}

double FaceRecognizerEigenfaces::GetSimilarity_ThirdMethod_old(const cv::Mat &firstImage, const cv::Mat &secondImage)
{
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

        cv::imshow("temporary1",temporary1);
        cv::imshow("temporary2",temporary2);
        cv::imshow("firstImage",firstImage);
        cv::imshow("secondImage",secondImage);
        cv::waitKey(0);

    double err = 0;
    for(int row = 0; row < diffMat.rows; row++)
    {
        unsigned char* points  = diffMat.ptr<unsigned char>(row);
        for (int col = 0; col < diffMat.cols; col++)
        {
            err += *points++;
        }
    }

    err /= ((double)diffMat.rows * (double)diffMat.cols);

    err *= 2.5;
    double prob = (1 - err);
    if (prob > 1) prob = 0.99;
    if (prob < 0) prob = 0.01;

    return prob;
}

//double FaceRecognizerEigenfaces::GetSimilarity_HashEigen(const cv::Mat &firstImage, const cv::Mat &secondImage){
//    cv::Mat temporary1, temporary2;
//    try
//    {
//        cv::cornerMinEigenVal(firstImage, temporary1, 10, 5);
//        cv::cornerMinEigenVal(secondImage, temporary2, 10, 5);
//    }
//    catch(...)
//    {
//        TRACE_F_T("Opencv failed to process cornerMinEigenVal function");
//        return -1;
//    }

//}


__int64_t FaceRecognizerEigenfaces::calcImageHash(cv::Mat &image)
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
__int64_t FaceRecognizerEigenfaces::calcHammingDistance(__int64_t firstHash, __int64_t secondHash)
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

FrokResult FaceRecognizerEigenfaces::GetImageHistogram(const cv::Mat &image, cv::MatND &histogram)
{
    TRACE_T("started");
    TRACE_W_T("This function works only for GrayScaled images");
    int channels = {0};     // Only 1 channel
    int histSize [] = {image.cols, image.cols};
    float range[] = {0, 256};
    const float* ranges [] = {range};     // full image

    try
    {
        cv::calcHist(&image, 1, &channels, cv::Mat(), histogram, 1, histSize, ranges, true, false);
        cv::normalize(histogram, histogram, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to calculate histogram");
        return FROK_RESULT_OPENCV_ERROR;
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

