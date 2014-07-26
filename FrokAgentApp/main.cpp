#include <stdio.h>

#include "FrokAgent.h"
//#include "FrokFaceDetector.h"
//#include "FrokFaceRecognizer.h"
//#include "FaceUserModel.h"
void usage()
{
    printf("FaceDetectionApp <Local port number>");
    return;
}

int main(void)
{
    /*if(!InitFaceCommonLib())
    {
        return -1;
    }
    FrokFaceRecognizer recognizer;
    FrokFaceDetector detector;
    FaceUserModel model((std::string)"1", RECOGNIZER_EIGENFACES);

    std::vector<cv::Rect> faces;
    std::vector<cv::Mat> images;

// Train Base
    timespec startTime;
    timespec endTime;
    memset(&startTime, 0, sizeof(startTime));
    memset(&endTime, 0, sizeof(endTime));
//START OF VERY FAST WAITING
    printf("Starting first image's face detection\n");
    clock_gettime(CLOCK_REALTIME, &startTime);

    detector.SetTargetImage("/home/zda/faces/1/photos/1.jpg");
    detector.GetFacesFromPhoto(faces);
    detector.GetNormalizedFaceImages(faces, images);

    clock_gettime(CLOCK_REALTIME, &endTime);
    print_time(startTime, endTime);

    printf("Starting first image's face detection\n");
    clock_gettime(CLOCK_REALTIME, &startTime);
    detector.SetTargetImage("/home/zda/faces/1/photos/2.jpg");
    detector.GetFacesFromPhoto(faces);
    detector.GetNormalizedFaceImages(faces, images);

    clock_gettime(CLOCK_REALTIME, &endTime);
    print_time(startTime, endTime);

    printf("Starting base generation\n");
    clock_gettime(CLOCK_REALTIME, &startTime);
    model.GenerateUserModel(images);
    recognizer.AddFrokUserModel((std::string)"1", model);

    clock_gettime(CLOCK_REALTIME, &endTime);
    print_time(startTime, endTime);
// Train Base finished

// Get recigized face
    printf("Face detection on recognized picture\n");
    clock_gettime(CLOCK_REALTIME, &startTime);

    detector.SetTargetImage("/home/zda/faces/1/photos/3.jpg");
    detector.GetFacesFromPhoto(faces);
    detector.GetNormalizedFaceImages(faces, images);

    clock_gettime(CLOCK_REALTIME, &endTime);
    print_time(startTime, endTime);

// Recognize
    printf("Recognition\n");
    clock_gettime(CLOCK_REALTIME, &startTime);

    std::vector<std::string> ids;
    ids.push_back("1");
    recognizer.SetUserIdsVector(ids);
    std::map<std::string, double> similarities;
    recognizer.GetSimilarityOfFaceWithModels(images[2], similarities);
    clock_gettime(CLOCK_REALTIME, &endTime);
    print_time(startTime, endTime);
    printf("done\n");*/

    /*for(std::vector<cv::Mat>::iterator it = images.begin(); it != images.end(); ++it)
    {
        cv::Mat face = (cv::Mat)*it;
        cv::imwrite("/home/zda/faces/face.jpg", face);
        //cv::imshow("image",);
    }*/


    getchar();

    return 0;
}
