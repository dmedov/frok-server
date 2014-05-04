#include <cv.h>
#include <highgui.h>

#include <windows.h>
#include <stdio.h>
#include <iostream> 


#include <opencv2/legacy/legacy.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

struct dataJson {
	vector<int> *ids = new vector<int>;
	vector<CvPoint> *p1s = new vector<CvPoint>, *p2s = new vector<CvPoint>;
	vector<double>*probs = new vector<double>;
};

struct imageCoordinats {
	CvPoint p1 = cvPoint(-1, -1);
	CvPoint p2 = cvPoint(0, 0);
};