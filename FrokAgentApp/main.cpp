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
    if(!InitFaceCommonLib())
    {
        return -1;
    }
    FrokFaceRecognizer recognizer;
    FrokFaceDetector detector;

    std::vector<std::string> ids;
    ids.push_back("0");
    /*ids.push_back("1");
    ids.push_back("2");
    ids.push_back("3");
    ids.push_back("4");
    ids.push_back("5");
    ids.push_back("6");
    ids.push_back("7");
    ids.push_back("8");
    ids.push_back("9");
    ids.push_back("10");
    ids.push_back("11");
    ids.push_back("12");
    ids.push_back("13");
    ids.push_back("14");
    ids.push_back("15");
    ids.push_back("16");
    ids.push_back("17");*/
    printf("Calling TrainUserModel...\n");
    TrainUserModel(ids, DEFAULT_PHOTO_BASE_PATH, detector, recognizer);
    printf("TrainUserModel finished\n");

    /*std::vector< std::map<std::string, double> > similarities;
    printf("Calling Recognize...\n");
    Recognize(similarities, ids, DEFAULT_PHOTO_BASE_PATH, "1.jpg", DEFAULT_TARGETS_FOLDER_PATH, detector, recognizer);
    printf("Rcognize finished\n");*/

    /*std::vector<cv::Rect> faces;
    std::vector<cv::Mat> images;
    detector.SetTargetImage("/home/zda/faces/0/photos/Picture (22) (Copy).jpg");
    detector.GetFacesFromPhoto(faces);
    detector.GetNormalizedFaceImages(faces, images);*/

    return 0;
}
