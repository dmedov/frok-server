#ifndef COMMON_H
#define COMMON_H
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

// Common defines
#define LOG_MESSAGE_MAX_LENGTH    1024

// Colored prints
#define _FAIL(__x__)    "\x1b[1;91m[FAIL] " __x__ "\n\x1b[0m"
#define _WARN(__x__)    "\x1b[1;93m[WARN] " __x__ "\n\x1b[0m"
#define _SUCC(__x__)    "\x1b[1;97m[SUCC] " __x__ "\n\x1b[0m"
#define _RES(__x__)     "\x1b[1;96m[RES] "  __x__ "\n\x1b[0m"
#define _N(__x__)                           __x__ "\n"

// Common externs
extern pthread_mutex_t fileCS;
extern pthread_mutex_t faceDetectionCS;
extern map < string, Ptr<FaceRecognizer> > models;
extern Network net;

// don't forget '/' in the end of dir name. Example: good = "/home/workspace/" bad: "/home/workspace"
int getFilesFromDir(const char *dir, vector<string> &files);
// Use colored prints - its cool
void FilePrintMessage(char* file, const char* expr...);
// Always call on your application start
void InitFaceDetectionLib();
// Call this on your application finish
void DeinitFaceDetectionLib();
#endif //COMMON_H
