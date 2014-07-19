#include "FaceDetector.h"

#define MODULE_NAME         "FACE_DETECTOR"

FaceDetector::FaceDetector()
{
    varScaleFactor = 1.1;
    varMinNeighbors = 3;
    varMinFaceSize = cvSize(40, 50);
    TRACE("new FaceDetector");
}

FaceDetector::~FaceDetector()
{
    TRACE("~FaceDetector");
}

FrokResult FaceDetector::SetFaceDetectionParameters(double scaleFactor, int minNeighbors, cv::Size minFaceSize)
{
    timespec startTime;
    timespec endTime;
    memset(&endTime, 0, sizeof(endTime));
    memset(&startTime, 0, sizeof(startTime));

    clock_gettime(CLOCK_REALTIME, &startTime);

    varScaleFactor = scaleFactor;
    varMinNeighbors = minNeighbors;
    varMinFaceSize = minFaceSize;
    clock_gettime(CLOCK_REALTIME, &endTime);
}

FrokResult FaceDetector::SetTargetImage(const char *imagePath)
{
    targetImageKappa = cv::imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
    if(!targetImageKappa.data)
    {
        TRACE_F_T("Failed to open image %s", imagePath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
}

FrokResult FaceDetector::GetFacesFromPhoto(std::vector< cv::Rect > &faces)
{
    try
    {
        cascades.face.detectMultiScale(targetImageKappa, faces, varScaleFactor, varMinNeighbors, 0, varMinFaceSize, cv::Size(targetImageKappa.cols, targetImageKappa.rows));
    }
    catch(...)
    {
        TRACE_F_T("detectMultiScale Failed");
        return FROK_RESULT_CASCADE_ERROR;
    }
    return FROK_RESULT_SUCCESS;
}
FrokResult FaceDetector::GetNormalizeFace(cv::Rect faceCoords, cv::Mat &normalizedFaceImage)
{}
FrokResult FaceDetector::GetFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages)
{}

/*delete []cascades;
for (int i = 0; i < MAX_THREADS_AND_CASCADES_NUM; i++)
{
    cvReleaseHaarClassifierCascade(&cascades[i].face->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].eyes->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].nose->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].mouth->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].eye->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].righteye2->oldCascade);
    cvReleaseHaarClassifierCascade(&cascades[i].lefteye2->oldCascade);
}*/

