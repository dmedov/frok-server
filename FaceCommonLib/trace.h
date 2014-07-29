#ifndef TRACE_H
#define TRACE_H

// include dependencies
#include <time.h>
#include <stdio.h>

#ifdef TRACE_DEBUG
// Colored print defines
#define _FAIL(__x__, ...)    "[%s->%s] \x1b[1;91m[FAIL] "   __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _WARN(__x__, ...)    "[%s->%s] \x1b[1;93m[WARN] "   __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _SUCC(__x__, ...)    "[%s->%s] \x1b[1;97m[SUCC] "   __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _RES(__x__, ...)     "[%s->%s] \x1b[1;95m[FAIL] "   __x__ "\n\x1b[0m", MODULE_NAME, __FUNCTION__, ##__VA_ARGS__
#define _N(__x__, ...)       "[%s->%s] "                    __x__ "\n",        MODULE_NAME, __FUNCTION__, ##__VA_ARGS__


#define TRACE_F_T(__x__, ...)   TRACE_TIMESTAMP(_FAIL(__x__,  ##__VA_ARGS__))
#define TRACE_W_T(__x__, ...)   TRACE_TIMESTAMP(_WARN(__x__,  ##__VA_ARGS__))
#define TRACE_S_T(__x__, ...)   TRACE_TIMESTAMP(_SUCC(__x__,  ##__VA_ARGS__))
#define TRACE_R_T(__x__, ...)   TRACE_TIMESTAMP(_RES(__x__, ##__VA_ARGS__))
#define TRACE_T(__x__, ...)     TRACE_TIMESTAMP(_N(__x__,  ##__VA_ARGS__))

#define TRACE_F(__x__, ...)     TRACE_PRINT(_FAIL(__x__,  ##__VA_ARGS__))
#define TRACE_W(__x__, ...)     TRACE_PRINT(_WARN(__x__,  ##__VA_ARGS__))
#define TRACE_S(__x__, ...)     TRACE_PRINT(_SUCC(__x__,  ##__VA_ARGS__))
#define TRACE_R(__x__, ...)     TRACE_PRINT(_RES(__x__, ##__VA_ARGS__))
#define TRACE(__x__, ...)       TRACE_PRINT(_N(__x__, ##__VA_ARGS__))

#define TRACE_PRINT(format...)          fprintf(stdout, ##format)

#define TRACE_TIMESTAMP(format...)      do{unsigned sec, usec;                      \
                                        set_time_stamp(&sec, &usec);                \
                                        fprintf(stdout, "[%5u.%06u]", sec, usec);   \
                                        fprintf(stdout, ##format);}while(0)
#else
#define TRACE_F_T(__x__, ...)
#define TRACE_W_T(__x__, ...)
#define TRACE_S_T(__x__, ...)
#define TRACE_R_T(__x__, ...)
#define TRACE_T(__x__, ...)
#define TRACE_F(__x__, ...)
#define TRACE_W(__x__, ...)
#define TRACE_S(__x__, ...)
#define TRACE_R(__x__, ...)
#define TRACE(__x__, ...)
#endif //TRACE_DEBUG
#endif // TRACE_H
