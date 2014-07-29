#ifndef FACEDETECTORABSTARCT_H
#define FACEDETECTORABSTARCT_H

// include dependencies
#include "faceCommonLib.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>

class FaceDetectorAbstract
{
protected:
    cv::Mat                 targetImageKappa;
public:
    FaceDetectorAbstract();
    virtual ~FaceDetectorAbstract();
    virtual FrokResult SetTargetImage(cv::Mat &image) = 0;
    virtual FrokResult SetTargetImage(const char *imagePath, bool dontResize = false) = 0;
    virtual FrokResult GetFacesFromPhoto(std::vector< cv::Rect > &faces) = 0;
    virtual FrokResult GetFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages) = 0;
    virtual FrokResult GetNormalizedFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages) = 0;
};

#endif // FACEDETECTORABSTARCT_H
