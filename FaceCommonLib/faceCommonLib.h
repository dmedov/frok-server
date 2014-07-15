#ifndef FACECOMMONLIB_H
#define FACECOMMONLIB_H

// Lib includes
#include "json.h"               // class for easy json (de)serializing, MIT LICENSE
#include "commonThread.h"       // class for safe working with unix threads
//#include "network.h"            // class for making tcp network server or client
#include "io.h"                 // File system - depend operations, input - output operations
// FaceDetectionLib defaults
#define LOG_MESSAGE_MAX_LENGTH      1024
#define DEFAULT_LOG_FILE_NAME       "facelog.log"

// Externals
extern pthread_mutex_t filePrint_cs;

// Common defines
#define UNREFERENCED_PARAMETER(P)           (P=P)
#define COMMAND_WITH_LENGTH(__CHARS__)      (__CHARS__), strlen((__CHARS__))

// Colored print defines
#define _FAIL(__x__)    "\x1b[1;91m[FAIL] " __x__ "\n\x1b[0m"
#define _WARN(__x__)    "\x1b[1;93m[WARN] " __x__ "\n\x1b[0m"
#define _SUCC(__x__)    "\x1b[1;97m[SUCC] " __x__ "\n\x1b[0m"
#define _RES(__x__)     "\x1b[1;96m[RES] "  __x__ "\n\x1b[0m"
#define _N(__x__)                           __x__ "\n"

// Library init functions
void InitFaceCommonLib(char *logFileName = DEFAULT_LOG_FILE_NAME);
void DeinitFaceCommonLib();

#endif // FACECOMMONLIB_H
