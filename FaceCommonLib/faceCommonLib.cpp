#include <pthread.h>
#include <string.h>
#include "faceCommonLib.h"

pthread_mutex_t filePrint_cs;

void InitFaceCommonLib(char *log_name)
{
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&filePrint_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);
    log_file = new char[strlen(log_name) + 1];
    strcpy(log_file, log_name);
}

void DeinitFaceCommonLib()
{
    delete log_file;
    pthread_mutex_destroy(&filePrint_cs);
}
