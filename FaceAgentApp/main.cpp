#include <stdio.h>

#include "FaceAgent.h"
#include "activities.h"
#include "FaceDetector.h"

void usage()
{
    printf("FaceDetectionApp <Local port number>");
    return;
}

int main(void)
{
    if(!InitFaceCommonLib())
    {
        return -1;
    }
    FaceDetector detector;
    detector.SetTargetImage("/home/zda/faces/1.jpg");
    std::vector<cv::Rect> faces;
    std::vector<cv::Mat> images;
    detector.GetFacesFromPhoto(faces);
    detector.GetNormalizedFaceImages(faces, images);

    for(std::vector<cv::Mat>::iterator it = images.begin(); it != images.end(); ++it)
    {
        cv::Mat face = (cv::Mat)*it;
        cv::imwrite("/home/zda/faces/face.jpg", face);
        //cv::imshow("image",);
    }
    getchar();

    return 0;
}
