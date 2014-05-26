#pragma once

typedef struct
{
	vector<char*> ids;
	vector<CvPoint> p1s, p2s;
	vector<double> probs;
} DataJson;

typedef struct StructImageCoordinats
{
	CvPoint p1;
	CvPoint p2;
	public:
	StructImageCoordinats()
	{
		p1 = cvPoint(-1, -1);
		p2 = cvPoint(0, 0);
	}
} ImageCoordinats;

#define LOG_MESSAGE_MAX_LENGTH	1024

#define _FAIL(__x__) "[FAIL] " __x__ "\n"
#define _WARN(__x__) "[WARN] " __x__ "\n"
#define _SUCC(__x__) "[SUCC] " __x__ "\n"
#define _RES(__x__) "[RES] " __x__ "\n"
#define _N(__x__) __x__ "\n"

void FilePrintMessage(char* file, char* expr...);
void ChooseTextColor(char* msg);
void RestoreTextColor();
extern HANDLE			hStdHandle;
extern CRITICAL_SECTION fileCS;
extern CRITICAL_SECTION faceDetectionCS;

void InitFaceDetectionLib();
void DeinitFaceDetectionLib();