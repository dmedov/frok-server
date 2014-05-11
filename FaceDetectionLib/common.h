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

#define LOG_MESSAGE_MAX_LENGTH	1024

#define _FAIL(__x__) "[FAIL] " __x__ "\n"
#define _WARN(__x__) "[WARN] " __x__ "\n"
#define _SUCC(__x__) "[SUCC] " __x__ "\n"

void FilePrintMessage(char* file, char* expr...);
void ChooseTextColor(char* msg);
void RestoreTextColor();
extern HANDLE			hStdHandle;