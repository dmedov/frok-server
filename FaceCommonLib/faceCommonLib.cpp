#include <pthread.h>
#include <string.h>
#include "faceCommonLib.h"

pthread_mutex_t filePrint_cs;

bool InitFaceCommonLib(const char *log_name)
{
    int result = 0;

    pthread_mutexattr_t mAttr;
    if(0 != (result = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP)))
    {
        printf("pthread_mutexattr_settype failed on error %s", strerror(result));
        return false;
    }

    if(0 != (result = pthread_mutex_init(&filePrint_cs, &mAttr)))
    {
        printf("pthread_mutex_init failed on error %s", strerror(result));
        return false;
    }

    if(0 != (result = pthread_mutexattr_destroy(&mAttr)))
    {
        printf("pthread_mutexattr_destroy failed on error %s", strerror(result));
        return false;
    }

    if(log_name != NULL)
    {
        pthread_mutex_lock(&filePrint_cs);
        if(log_file != NULL)
        {
            delete []log_file;
        }
        log_file = new char[strlen(log_name) + 1];
        strcpy(log_file, log_name);
        pthread_mutex_unlock(&filePrint_cs);
    }

    return true;
}

bool DeinitFaceCommonLib()
{
    int result = 0;
    if(log_file != NULL)
    {
        delete []log_file;
    }

    if(0 != (result = pthread_mutex_destroy(&filePrint_cs)))
    {
        printf("pthread_mutexattr_destroy failed on error %s", strerror(result));
        return false;
    }
    return true;
}
