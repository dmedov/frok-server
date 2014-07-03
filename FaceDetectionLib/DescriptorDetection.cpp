#include "stdafx.h"
#include "DescriptorDetection.h"
#include "LibInclude.h"


int DescriptorDetection::matchDescriptors(Mat &ffd, Mat &bfd){
    FlannBasedMatcher matcher;
    vector<DMatch> matches;
    matcher.match(ffd, bfd, matches);

    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for (int i = 0; i < ffd.rows; i++)    {
        double dist = matches[i].distance;
        if (dist < min_dist) min_dist = dist;
        if (dist > max_dist) max_dist = dist;
    }

    vector< DMatch > good_matches;
    int c = 0;
    for (int i = 0; i < ffd.rows; i++)    {
        if (matches[i].distance <= 2 * min_dist)    {
            good_matches.push_back(matches[i]);
            c++;
        }
    }
    FilePrintMessage(NULL, _N("matchDescriptors: c = %d"), c);
    return c;
}

Mat DescriptorDetection::findDescriptors(Mat &face, char* name, bool oShowImage){
    //FeatureDetector * detector = new GFTTDetector();
    //FeatureDetector * detector = new FastFeatureDetector();
    //FeatureDetector * detector = new DenseFeatureDetector(1, 1, 0.1, 6,0, true, false);
    //FeatureDetector * detector = new StarFeatureDetector();
    //FeatureDetector * detector = new BRISK();
    //FeatureDetector * detector = new MSER();
    //FeatureDetector * detector = new ORB();
    FeatureDetector * detector = new SURF(600.0);
    //FeatureDetector * detector = new SIFT(600);

    vector<KeyPoint> keypoints;
    detector->detect(face, keypoints);
    Mat img_keypoints, descriptors_object;
    drawKeypoints(face, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    SiftDescriptorExtractor extractor;
    extractor.compute(face, keypoints, descriptors_object);
    if (oShowImage)
    {
        imshow(name, img_keypoints);
    }
    return descriptors_object;
}

