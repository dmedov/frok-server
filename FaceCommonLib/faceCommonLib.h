#ifndef FACECOMMONLIB_H
#define FACECOMMONLIB_H

// Common includes
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

// Result of Face*Libraries
typedef enum FrokResult
{
    FROK_RESULT_NULL,
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

// Common defaults
#define FROK_LIB_COMMON_DEFAULT_CONFIG_FILENAME "/etc/frok/frok.conf"
#define FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME "/var/log/frok.log"

// Common defines
#define COMMAND_WITH_LENGTH(__CHARS__)      (__CHARS__), strlen((__CHARS__))
#define CASE_RET_STR(x)                     case x: return #x

#define BOOL                                unsigned char
#define TRUE                                1
#define FALSE                               0

#pragma pack(push, 1)
typedef struct frokCommonContext
{
    char *outputFile;
    pthread_mutex_t common_cs;
    pthread_mutex_t trace_cs;
    struct timespec startTime;
}frokCommonContext;
#pragma pack(pop)

extern frokCommonContext *commonContext;


// Lib includes
//#include "commonThread.h"       // class for safe working with unix threads
#include "io.h"                 // File system - depend operations, input - output operations
#include "commonMath.h"         // Calculating ChiSquare percantage
#include "linux/linuxDefines.h"
#include "linux/commonSched.h"

#ifdef __cplusplus
extern "C" {
#endif

// Library init functions
const char *FrokResultToString(FrokResult res);
FrokResult frokLibCommonInit(const char *configFilePath);
void frokLibCommonDeinit();
void set_time_stamp(unsigned *sec, unsigned *usec);
void set_trace_prefix(const char *prefix);
#ifdef __cplusplus
}
#endif

#endif // FACECOMMONLIB_H
