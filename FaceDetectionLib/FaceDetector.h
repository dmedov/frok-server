#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H
// FaceServer defaults
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "faceCommonLib.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>


// FaceAgentConnector logging system
// [TBD] Add file printing and timestamps
#ifdef FACE_DETECTOR_TRACE_ENABLED
#define FACE_DETECTOR_TRACE(__function_name__, format, ...)     \
    pthread_mutex_lock(&faceDetector_trace_cs);                 \
    printf("[FACE_DETECTOR->%s]: ", #__function_name__);        \
    printf(format, ##__VA_ARGS__);                              \
    printf("\n");                                               \
    pthread_mutex_unlock(&faceDetector_trace_cs)
#else
#define FACE_DETECTOR_TRACE(__function_name__, format, ...)
#endif

// [TBD] somehow replace paths to haarcascades with path defines
struct HaarCascades
{
    cv::CascadeClassifier *face;
    cv::CascadeClassifier *eyes;
    cv::CascadeClassifier *righteye;
    cv::CascadeClassifier *lefteye;
    cv::CascadeClassifier *righteye2;
    cv::CascadeClassifier *lefteye2;
    cv::CascadeClassifier *eye;
    cv::CascadeClassifier *nose;
    cv::CascadeClassifier *mouth;
    HaarCascades()
    {
        face        = new cv::CascadeClassifier;
        eyes        = new cv::CascadeClassifier;
        righteye    = new cv::CascadeClassifier;
        lefteye     = new cv::CascadeClassifier;
        righteye2   = new cv::CascadeClassifier;
        lefteye2    = new cv::CascadeClassifier;
        eye         = new cv::CascadeClassifier;
        mouth       = new cv::CascadeClassifier;
        nose        = new cv::CascadeClassifier;
        face->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml");
        eyes->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml");
        righteye->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_righteye.xml");
        lefteye->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_lefteye.xml");
        righteye2->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_righteye_2splits.xml");
        lefteye2->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_lefteye_2splits.xml");
        eye->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye.xml");
        nose->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_nose.xml");
        mouth->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml");
    }
    ~HaarCascades()
    {
        delete face;
        delete eyes;
        delete righteye;
        delete lefteye;
        delete righteye2;
        delete lefteye2;
        delete eye;
        delete mouth;
        delete nose;
    }
};

class FaceDetector
{
public:
    // Parameter specifying how much the image size is reduced at each image scale
    double varScaleFactor;
    // Parameter specifying how many neighbors each candidate rectangle should have to retain it.
    int varMinNeighbors;
    // Minimum possible object size. Objects smaller than that are ignored.
    cv::Size varMinFaceSize;
private:
    HaarCascades cascades;

    cv::Mat         targetImageOrigin;
    cv::Mat         targetImageKappa;
public:
    FaceDetector();
    ~FaceDetector();
    FrokResult SetFaceDetectionParameters(double scaleFactor, int minNeighbors, cv::Size minFaceSize);
    FrokResult SetTargetImage(const char *imagePath);
    FrokResult SetTargetImage(cv::Mat &image);
    FrokResult GetFacesFromPhoto(std::vector< cv::Rect > &faces);
    FrokResult GetNormalizeFace(cv::Rect faceCoords, cv::Mat &normalizedFaceImage);
    FrokResult GetFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages);
private:

};
#endif
