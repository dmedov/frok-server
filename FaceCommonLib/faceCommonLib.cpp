#include <pthread.h>

#include "faceCommonLib.h"

pthread_mutex_t filePrint_cs;

void InitFaceCommonLib()
{
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&filePrint_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);
}

void DeinitFaceCommonLib()
{
    pthread_mutex_destroy(&filePrint_cs);
}
