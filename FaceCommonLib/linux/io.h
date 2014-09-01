#ifndef IO_H
#define IO_H

#include "faceCommonLib.h"

// include dependencies
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#pragma GCC poison cout

// Trace system
#ifdef TRACE_DEBUG

// Colored print defines
#define _FAIL(__x__, ...)    "[%s->%s] \x1b[1;91m[FAIL] "   __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _WARN(__x__, ...)    "[%s->%s] \x1b[1;93m[WARN] "   __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _SUCC(__x__, ...)    "[%s->%s] \x1b[1;97m[SUCC] "   __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _RES(__x__, ...)     "[%s->%s] \x1b[1;95m[RES] "    __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _N(__x__, ...)       "[%s->%s] "                    __x__ "\n",        MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#else
#define _FAIL(__x__, ...)    "[%s->%s] [FAIL] "   __x__ "\n", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _WARN(__x__, ...)    "[%s->%s] [WARN] "   __x__ "\n", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _SUCC(__x__, ...)    "[%s->%s] [SUCC] "   __x__ "\n", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _RES(__x__, ...)     "[%s->%s] [RES] "    __x__ "\n", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _N(__x__, ...)       "[%s->%s] "          __x__ "\n", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#endif //TRACE_DEBUG

#define TRACE_F_T(__x__, ...)   TRACE_TIMESTAMP(_FAIL(__x__,  ##__VA_ARGS__))
#define TRACE_W_T(__x__, ...)   TRACE_TIMESTAMP(_WARN(__x__,  ##__VA_ARGS__))
#define TRACE_S_T(__x__, ...)   TRACE_TIMESTAMP(_SUCC(__x__,  ##__VA_ARGS__))
#define TRACE_R_T(__x__, ...)   TRACE_TIMESTAMP(_RES(__x__, ##__VA_ARGS__))
#define TRACE_T(__x__, ...)     TRACE_TIMESTAMP(_N(__x__,  ##__VA_ARGS__))

#define TRACE_F(__x__, ...)     TRACE(_FAIL(__x__,  ##__VA_ARGS__))
#define TRACE_W(__x__, ...)     TRACE(_WARN(__x__,  ##__VA_ARGS__))
#define TRACE_S(__x__, ...)     TRACE(_SUCC(__x__,  ##__VA_ARGS__))
#define TRACE_R(__x__, ...)     TRACE(_RES(__x__, ##__VA_ARGS__))
#define TRACE_N(__x__, ...)     TRACE(_N(__x__, ##__VA_ARGS__))

#define TRACE_TIMESTAMP(format...)      do{struct timespec ts;                                  \
                                        int ret;                                                \
                                        pthread_mutex_lock(&commonContext->trace_cs);           \
                                        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);           \
                                        fprintf(stdout, "[%5li.%09li]", ts.tv_sec, ts.tv_nsec); \
                                        fprintf(stdout, ##format);                              \
                                        pthread_mutex_unlock(&commonContext->trace_cs);}while(0)

#define TRACE(format...)                fprintf(stdout, ##format)

//externals

#ifdef __cplusplus
extern "C" {
#endif

// don't forget '/' in the end of dir name.
// Example: good = "/home/workspace/" bad: "/home/workspace"
BOOL getFilesFromDir(const char *dir, char ***files, unsigned *filesNum);
BOOL getSubdirsFromDir(const char *dir, char ***files, unsigned *filesNum);

#ifdef __cplusplus
}
#endif

#endif // IO_H
