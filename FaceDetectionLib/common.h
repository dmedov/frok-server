#pragma once

struct DataJson {
	vector<int> *ids = new vector<int>;
	vector<CvPoint> *p1s = new vector<CvPoint>, *p2s = new vector<CvPoint>;
	vector<double>*probs = new vector<double>;
};

struct ImageCoordinats {
	CvPoint p1 = cvPoint(-1, -1);
	CvPoint p2 = cvPoint(0, 0);
};