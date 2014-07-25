#include <stdio.h>

#include "FaceAgent.h"
#include "activities.h"
#include "FrokFaceDetector.h"
#include "FrokFaceRecognizer.h"
#include "FaceUserModel.h"
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
    FrokFaceRecognizer recognizer;
    FrokFaceDetector detector;
    FaceUserModel model((std::string)"1", RECOGNIZER_EIGENFACES);
    cv::Mat targetForRecognize = cv::imread("/home/zda/faces/1.jpg", cv::IMREAD_GRAYSCALE);
    detector.SetTargetImage("/home/zda/faces/1.jpg");
    std::vector<cv::Rect> faces;
    std::vector<cv::Mat> images;
    detector.GetFacesFromPhoto(faces);
    detector.GetNormalizedFaceImages(faces, images);

    model.GenerateUserModel(images);

    recognizer.AddFrokUserModel((std::string)"1", model);
    std::vector<std::string> ids;
    ids.push_back("1");
    recognizer.SetUserIdsVector(ids);

    std::map<std::string, double> similarities;

    recognizer.GetSimilarityOfFaceWithModels(images[0], similarities);

    printf("done\n");

    /*for(std::vector<cv::Mat>::iterator it = images.begin(); it != images.end(); ++it)
    {
        cv::Mat face = (cv::Mat)*it;
        cv::imwrite("/home/zda/faces/face.jpg", face);
        //cv::imshow("image",);
    }*/


    getchar();

    return 0;
}
