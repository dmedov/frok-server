#include <pthread.h>
#include <unistd.h>
#include "commonThread.h"

#define MODULE_NAME     "COMMON_THREAD"

CommonThread::CommonThread()
{
    if(0 != sem_init(&threadStoppedSema, 0, 0))
    {
        CTHREAD_PRINT(CommonThread, "sem_init failed on error %s", strerror(errno));
    }
    threadState = COMMON_THREAD_NOT_INITED;
}

CommonThread::~CommonThread()
{
    if(!((threadState == COMMON_THREAD_NOT_INITED) || (threadState == COMMON_THREAD_STOPPED) || (threadState == COMMON_THREAD_ERROR)))
    {
        if(!stopThread())
        {
            CTHREAD_PRINT(~CommonThread, "failed to stop thread");
        }
    }
    sem_destroy(&threadStoppedSema);
}

bool CommonThread::startThread(void *(*function) (void *), void *functionParams, unsigned functionParamsLength, void *pObject)
{
    if(!((threadState == COMMON_THREAD_NOT_INITED) || (threadState == COMMON_THREAD_STOPPED) || (threadState == COMMON_THREAD_ERROR)))
    {
        CTHREAD_PRINT(startThread, "invalid threadState %d", threadState);
        return false;
    }
    pthread_attr_t tAttr;
    memset(&tAttr, 0, sizeof(pthread_attr_t));

    if (0 != pthread_attr_init(&tAttr))
    {
        CTHREAD_PRINT(startThread, "pthread_attr_init failed on error %s", strerror(errno));
        return false;
    }

    //if (0 != pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_DETACHED))
    if (0 != pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_JOINABLE))
    {
        CTHREAD_PRINT(startThread, "pthread_attr_setdetachstate failed on error %s", strerror(errno));
        return false;
    }

    CommonThread *pThis = this;
    startRoutineParams sParams;

    if(pObject != NULL)
    {
        memcpy(&sParams.functionParameters.object, pObject, sizeof(void*));
    }

    memcpy(&sParams.functionParameters.thread, &pThis, sizeof(CommonThread*));
    sParams.functionParameters.paramsLength = functionParamsLength;
    sParams.functionParameters.params = functionParams;
    sParams.function = function;

    threadState = COMMON_THREAD_INITIATED;

    if(0 != sem_init(&sParams.threadStartedSema, 0, 0))
    {
        CTHREAD_PRINT(startThread, "sem_init failed on error %s", strerror(errno));
        threadState = COMMON_THREAD_NOT_INITED;
        return false;
    }

    if (0 != pthread_create(&thread, &tAttr, (void * (*)(void*))(CommonThread::startRoutine), &sParams))
    {
        CTHREAD_PRINT(startThread, "pthread_create failed on error %s", strerror(errno));
        threadState = COMMON_THREAD_NOT_INITED;
        return false;
    }

    timespec sTime;
    memset(&sTime, 0, sizeof(timespec));

    sTime.tv_sec = time(NULL);
    sTime.tv_sec += COMMON_THREAD_WAIT_TIMEOUT / 1000;

    if(0 != sem_timedwait(&sParams.threadStartedSema, &sTime))
    {
        CTHREAD_PRINT(startThread, "sem_timedwait failed on error %s", strerror(errno));
        pthread_cancel(thread);
        threadState = COMMON_THREAD_ERROR;
        return false;
    }

    threadState = COMMON_THREAD_STARTED;
    CTHREAD_PRINT(startThread, "Thread started");
    return true;
}

bool CommonThread::isStopThreadReceived()
{
    if(0 == sem_trywait(&threadStoppedSema))
    {
        return true;
    }
    return false;
}

bool CommonThread::stopThread()
{
    int ret_code = 0;

    if((threadState == COMMON_THREAD_STOPPED) || (threadState == COMMON_THREAD_NOT_INITED))
    {
        CTHREAD_PRINT(stopThread, "already stopped");
        return true;
    }

    CTHREAD_PRINT(stopThread, "Stopping thread");

    if(0 != sem_post(&threadStoppedSema))
    {
        CTHREAD_PRINT(stopThread, "sem_post failed on error %s", strerror(errno));
        return false;
    }

// [TBD] :: add portable versions
    timespec sTime;
    memset(&sTime, 0, sizeof(timespec));

    sTime.tv_sec = time(NULL);
    sTime.tv_sec += COMMON_THREAD_WAIT_TIMEOUT / 1000;

    if(0 != (ret_code = pthread_timedjoin_np(thread, NULL, &sTime)))
    {
        CTHREAD_PRINT(stopThread, "pthread_timedjoin_np failed on error %s", strerror(ret_code));
        CTHREAD_PRINT(stopThread, "Calling pthread_cancel");
        if(0 != pthread_cancel(thread))
        {
            CTHREAD_PRINT(stopThread, "pthread_cancel failed on error %s", strerror(ret_code));
        }

        CTHREAD_PRINT(stopThread, "Something failed. Starting infinite waiting for thread finished...");
        if(0 != (ret_code = pthread_join(thread, NULL)))
        {
            CTHREAD_PRINT(stopThread, "pthread_join failed on error %s", strerror(ret_code));
            threadState = COMMON_THREAD_ERROR;
            return false;
        }
        else
        {
            CTHREAD_PRINT(stopThread, "Thread successfullty cancelled.");
        }
    }

    CTHREAD_PRINT(stopThread, "Thread stopped");

    threadState = COMMON_THREAD_STOPPED;

    return true;
}

void CommonThread::startRoutine(void *param)
{
    CTHREAD_PRINT(startRoutine, "Thread routine started.");
    startRoutineParams *psParams = (startRoutineParams*) param;

    ThreadFunctionParameters thParams;
    memset(&thParams, 0, sizeof(thParams));
    thParams.object = psParams->functionParameters.object;
    //memcpy(&thParams.thread, &psParams->functionParameters.thread);
    thParams.thread = psParams->functionParameters.thread;
    thParams.paramsLength = psParams->functionParameters.paramsLength;
    if(thParams.paramsLength != 0)
    {
        thParams.params = new char [thParams.paramsLength];
        memcpy(thParams.params, psParams->functionParameters.params, thParams.paramsLength);
    }
    void *(*function) (void *) = psParams->function;

    sem_post(&psParams->threadStartedSema);

    CTHREAD_PRINT(startRoutine, "User function started.");
    function(&thParams);
    CTHREAD_PRINT(startRoutine, "User function finished.");

    thParams.thread->threadState = COMMON_THREAD_NOT_INITED;

    if(thParams.params != NULL)
    {
        delete [](char*)thParams.params;
    }
    CTHREAD_PRINT(startRoutine, "Thread routine finished.");
}

commonThreadState CommonThread::getThreadState()
{
    return threadState;
}
