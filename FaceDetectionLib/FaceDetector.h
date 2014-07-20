#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H
// FaceServer defaults
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "faceCommonLib.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>

// [TBD] somehow replace paths to haarcascades with path defines
struct HaarCascades
{
    cv::CascadeClassifier face;
    cv::CascadeClassifier eyeglasses;
    cv::CascadeClassifier righteye;
    cv::CascadeClassifier lefteye;
    cv::CascadeClassifier splitted_righteye;
    cv::CascadeClassifier splitted_lefteye;
    cv::CascadeClassifier eye;
    cv::CascadeClassifier mcs_nose;
    cv::CascadeClassifier msc_mouth;
    HaarCascades()
    {
        face.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml");
        eyeglasses.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml");
        righteye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_righteye.xml");
        lefteye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_lefteye.xml");
        splitted_righteye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_righteye_2splits.xml");
        splitted_lefteye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_lefteye_2splits.xml");
        eye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye.xml");
        mcs_nose.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_nose.xml");
        msc_mouth.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml");
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
    //cv::Mat         targetImageOrigin; we don't need not gray-scaled photo now
    cv::Mat         targetImageKappa;

public:
    FaceDetector();
    ~FaceDetector();
    FrokResult SetFaceDetectionParameters(double scaleFactor, int minNeighbors, cv::Size minFaceSize);
    FrokResult SetTargetImage(const char *imagePath);
    FrokResult SetTargetImage(cv::Mat &image);
    FrokResult GetFacesFromPhoto(std::vector< cv::Rect > &faces);
    FrokResult GetFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages);
private:
    FrokResult NormalizeFace(cv::Mat &normalizedFaceImage);
    FrokResult RemoveDrowbackFrokImage(cv::Mat &image);
    FrokResult RotateImage(cv::Mat &image);
};
#endif
