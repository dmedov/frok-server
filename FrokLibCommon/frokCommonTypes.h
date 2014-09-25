#ifndef FROKCOMMONTYPES_H
#define FROKCOMMONTYPES_H

#include <pthread.h>

// Result of Face*Libraries
typedef enum FrokResult
{
    // Returns of Agent
    FROK_RESULT_NULL,
    FROK_RESULT_SUCCESS,
    FROK_RESULT_UNSPECIFIED_ERROR,
    FROK_RESULT_LINUX_ERROR,
    FROK_RESULT_INVALID_STATE,
    FROK_RESULT_SOCKET_ERROR,
    FROK_RESULT_MEMORY_FAULT,
    // Others
    FROK_RESULT_INVALID_PARAMETER,
    FROK_RESULT_NO_FACES_FOUND,
    FROK_RESULT_OPENCV_ERROR,
    FROK_RESULT_NO_MODELS,
    FROK_RESULT_PERM_ERROR,
    // etc
} FrokResult;

#define BOOL                                unsigned char
#define TRUE                                1
#define FALSE                               0
#define SOCKET                              int
#define SOCKET_ERROR                        (-1)
#define INVALID_SOCKET                      SOCKET_ERROR

#define UNUSED(P)                           __attribute__ ((unused)) P


#pragma pack(push, 1)
typedef struct frokCommonContext
{
    char *outputFile;
    char *photoBasePath;
    char *targetPhotosPath;
    pthread_mutex_t common_cs;
    pthread_mutex_t trace_cs;
    struct timespec startTime;
}frokCommonContext;
#pragma pack(pop)

#endif // FROKCOMMONTYPES_H
