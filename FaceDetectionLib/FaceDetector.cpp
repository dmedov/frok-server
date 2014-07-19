#include "FaceDetector.h"

FaceDetector::FaceDetector()
{
    varScaleFactor = 1.1;
    varMinNeighbors = 3;
    varMinFaceSize = cvSize(40, 50);
    FACE_DETECTOR_TRACE(FaceDetector, "new FaceDetector");
}

FaceDetector::~FaceDetector()
{
    FACE_DETECTOR_TRACE(~FaceDetector, "~FaceDetector");
}

FrokResult FaceDetector::SetFaceDetectionParameters(double scaleFactor, int minNeighbors, cv::Size minFaceSize)
{
    varScaleFactor = scaleFactor;
    varMinNeighbors = minNeighbors;
    varMinFaceSize = minFaceSize;
}

FrokResult FaceDetector::SetTargetImage(const char *imagePath)
{
    targetImageOrigin = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);
    targetImageKappa = cv::imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
}

FrokResult FaceDetector::GetFacesFromPhoto(std::vector< cv::Rect > &faces)
{
    try
    {
        cascades.face->detectMultiScale(targetImageKappa, faces, varScaleFactor, varMinNeighbors, 0, varMinFaceSize, cv::Size(targetImageKappa.cols, targetImageKappa.rows));
    }
    catch(...)
    {
        FACE_DETECTOR_TRACE(GetFacesFromPhoto, "detectMultiScale Failed");
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

