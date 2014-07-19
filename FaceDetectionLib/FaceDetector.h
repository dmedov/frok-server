#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H
// FaceServer defaults
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"
// opencv dependencies
#include <cv.h>
#include <highgui.h>


// FaceAgentConnector logging system
// [TBD] Add file printing and timestamps
#ifdef FACE_DETECTOR_TRACE_ENABLED
#define FACE_DETECTOR_TRACE(__function_name__, format, ...)    \
    pthread_mutex_lock(&faceDetector_trace_cs);                \
    printf("[FACE_DETECTOR->%s]: ", #__function_name__);       \
    printf(format, ##__VA_ARGS__);                          \
    printf("\n");                                           \
    pthread_mutex_unlock(&faceDetector_trace_cs)
#else
#define FACE_DETECTOR_TRACE(__function_name__, format, ...)
#endif

class FaceDetector
{
private:
    IplImage   *iiTargetImageOrigin;
    Mat        *mTargetImageOrigin;
    IplImage   *iiTargetImageKappa;
    Mat        *mTargetImageKappa;
public:
    FrokResult SetTargetImage(const char *imagePath);
    FrokResult GetFacesFromPhoto(std::vector<cv::Rect> *faces);
    FrokResult NormalizeFace(cv::Rect faceCoords);
private:

};
#endif
