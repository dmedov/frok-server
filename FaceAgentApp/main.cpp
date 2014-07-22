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
    detector.GetFaceImages(faces, images);


    for(std::vector<cv::Mat>::iterator it = images.begin(); it != images.end(); ++it)
    {
        cv::Mat face = (cv::Mat)*it;
        detector.AlignFaceImage(face);
        cv::imwrite("/home/zda/faces/face.jpg", (cv::Mat)*it);
        //cv::imshow("image",);
    }
    getchar();
    //detector.SetTargetImage()
    /*FaceAgent agent(DEFAULT_PORT);
    agent.StartFaceAgent();
    getchar();

    agent.StopFaceAgent();*/

    return 0;
}
