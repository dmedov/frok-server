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
    varScaleFactor = scaleFactor;
    varMinNeighbors = minNeighbors;
    varMinFaceSize = minFaceSize;
}

FrokResult FaceDetector::SetTargetImage(const char *imagePath)
{
    TRACE_T("started...");
    targetImageKappa = cv::imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
    if(!targetImageKappa.data)
    {
        TRACE_F_T("Failed to open image %s", imagePath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    TRACE_T("finished");
}

FrokResult FaceDetector::GetFacesFromPhoto(std::vector< cv::Rect > &faces)
{
    TRACE_T("started...");
    try
    {
        cascades.face.detectMultiScale(targetImageKappa, faces, varScaleFactor, varMinNeighbors, 0, varMinFaceSize, cv::Size(targetImageKappa.cols, targetImageKappa.rows));
    }
    catch(...)
    {
        TRACE_F_T("detectMultiScale Failed");
        return FROK_RESULT_CASCADE_ERROR;
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}
FrokResult FaceDetector::GetFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages)
{
    TRACE_T("started...");

    if(coords.empty())
    {
        return FROK_RESULT_INVALID_PARAMETER;
    }
    for(std::vector<cv::Rect>::iterator it = coords.begin(); it != coords.end(); ++it)
    {
        cv::Rect faceCoord = (cv::Rect)*it;
        cv::Mat faceImage(targetImageKappa, faceCoord);
        faceImages.push_back(faceImage);
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceDetector::NormalizeFace(cv::Mat &normalizedFaceImage)
{
    TRACE_T("started...");
    FrokResult res;
    if(FROK_RESULT_SUCCESS != (res = RotateImage(normalizedFaceImage)))
    {
        TRACE_F_T("RotateImage failed on result %x", res);
        return res;
    }

    if(FROK_RESULT_SUCCESS != (res = RemoveDrowbackFrokImage(normalizedFaceImage)))
    {
        TRACE_F_T("RemoveDrowbackFrokImage failed on result %x", res);
        return res;
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceDetector::RotateImage(cv::Mat &image)
{
    double rad = 57.295779513;
    CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);

    if ((facePoints[0].x > 0 && facePoints[0].y > 0) && (facePoints[1].x > 0 && facePoints[1].y > 0))
    {
        int w = face_img->width;
        int h = face_img->height;

        CvPoint pa = cvPoint((facePoints[0].x + facePoints[4].x) / 2, (facePoints[0].y + facePoints[4].y) / 2);
        CvPoint pb = cvPoint((facePoints[1].x + facePoints[5].x) / 2, (facePoints[1].y + facePoints[5].y) / 2);

        double x = (pb.x - pa.x);
        double y = (pb.y - pa.y);
        CvPoint2D32f center;

        center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);

        double angle = atan(y / x)*rad;

        cv2DRotationMatrix(center, angle, 1, transmat);
        cvWarpAffine(face_img, face_img, transmat);
        cvReleaseMat(&transmat);
        return 0;
    }

    else if ((facePoints[2].x > 0 && facePoints[2].y > 0) && (facePoints[3].x > 0 && facePoints[3].y > 0)){

        int w = face_img->width;
        int h = face_img->height;

        CvPoint pa = cvPoint((facePoints[2].x + facePoints[6].x) / 2, (facePoints[2].y + facePoints[6].y) / 2);
        CvPoint pb = cvPoint((facePoints[3].x + facePoints[7].x) / 2, (facePoints[3].y + facePoints[7].y) / 2);

        CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);
        double x = (pb.x - pa.x);
        double y = (pb.y - pa.y);
        CvPoint2D32f center;

        center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);
        double angle = 0;
        angle = atan(y / x)*rad;

        if (abs(angle) > 30){
            cvReleaseMat(&transmat);
            return -1;
        }
        if (angle > 0)  angle -= 90;
        else if (angle < 0) angle += 90;
        else angle = 90;

        cv2DRotationMatrix(center, angle, 1, transmat);
        cvWarpAffine(face_img, face_img, transmat);
        cvReleaseMat(&transmat);
        return 0;
    }
}

FrokResult FaceDetector::RemoveDrowbackFrokImage(cv::Mat &image)
{}
