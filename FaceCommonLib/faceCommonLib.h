#ifndef FACECOMMONLIB_H
#define FACECOMMONLIB_H

// Lib includes
//#include "commonThread.h"       // class for safe working with unix threads
#include "io.h"                 // File system - depend operations, input - output operations
#include "commonMath.h"         // Calculating ChiSquare percantage
#include "linux/linuxDefines.h"
// Common includes
#include <time.h>
#include <pthread.h>

// Result of Face*Libraries
typedef enum FrokResult
{
    FROK_RESULT_SUCCESS,
    FROK_RESULT_CASCADE_ERROR,
    FROK_RESULT_UNSPECIFIED_ERROR,
    FROK_RESULT_NOT_A_FACE,
    FROK_RESULT_INVALID_PARAMETER,
    FROK_RESULT_OPENCV_ERROR,
    FROK_RESULT_NO_MODELS,
    FROK_RESULT_LINUX_ERROR,
    FROK_RESULT_INVALID_STATE,
    FROK_RESULT_SOCKET_ERROR,
    FROK_RESULT_MEMORY_FAULT
    // etc
} FrokResult;

// Common defines
#define UNREFERENCED_PARAMETER(P)           (P=P)

#define COMMAND_WITH_LENGTH(__CHARS__)      (__CHARS__), strlen((__CHARS__))
#define CASE_RET_STR(x)                     case x: return #x

#define BOOL                                unsigned char
#define TRUE                                1
#define FALSE                               0

#ifdef __cplusplus
extern "C" {
#endif

// Library init functions
const char *FrokResultToString(FrokResult res);
FrokResult InitFaceCommonLib(const char *log_name);
FrokResult DeinitFaceCommonLib();
void set_time_stamp(unsigned *sec, unsigned *usec);
//void print_time(timespec startTime, timespec endTime);

#ifdef __cplusplus
}
#endif

#endif // FACECOMMONLIB_H
