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
    //std::vector<cv::Mat> images;
    detector.GetFacesFromPhoto(faces);
    //detector.GetFaceImages(faces, images);


    for(std::vector<cv::Rect>::iterator it = faces.begin(); it != faces.end(); ++it)
    {
        cv::Mat face;
        detector.AlignFaceImage(*it, face);
        cv::imwrite("/home/zda/faces/face.jpg", face);
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
