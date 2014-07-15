#ifndef COMMONTHREAD_H
#define COMMONTHREAD_H

// Include dependencies
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef COMMON_THREAD_PRINT_ENABLED
#define CTHREAD_PRINT(__function_name__, format, ...)           \
    printf("[COMMON_THREAD] [%s]: ", #__function_name__);       \
    printf(format, ##__VA_ARGS__);                              \
    printf("\n")
#else
#define CTHREAD_PRINT(format, ...)
#endif

#define COMMON_THREAD_WAIT_TIMEOUT      (10000)

typedef enum EnumCommonThreadState
{
    COMMON_THREAD_NOT_INITED   =   0x00,
    COMMON_THREAD_INITIATED    =   0x01,
    COMMON_THREAD_STARTED      =   0x02,
    COMMON_THREAD_STOPPED      =   0x03,
    COMMON_THREAD_ERROR        =   0x04
} commonThreadState;

#pragma pack (push, 1)

template< class T >
struct startRoutineParamsTemp
{
    void           *pThis;
    sem_t          *threadStartedSema;
    unsigned        paramsLength;
    void           *params;
    void           *object;
    void (T::*virtualFunction)(void *);
    //void         *(*function) (void *);
    startRoutineParamsTemp()
    {
        threadStartedSema = new sem_t;
        if(0 != sem_init(threadStartedSema, 0, 0))
        {
            CTHREAD_PRINT(startRoutineParamsTemp, "sem_init failed on error %s", strerror(errno));
        }
    }
    ~startRoutineParamsTemp()
    {
        delete threadStartedSema;
    }
};

class CommonThread
{
private:
    commonThreadState   threadState;
    pthread_t          *thread;
    sem_t              *threadStoppedSema;

public:
    CommonThread();
    ~CommonThread();

    bool startThread(void *(*function) (void *), void *functionParams, unsigned functionParamsLength);
    template< class T >
    bool startVirtualFunctionThread(T& object, void (T::*virtualFunction)(void *), void *functionParams, unsigned functionParamsLength)
    {
        if(!((threadState == COMMON_THREAD_NOT_INITED) || (threadState == COMMON_THREAD_STOPPED) || (threadState == COMMON_THREAD_ERROR)))
        {
            CTHREAD_PRINT(startThread, "invalid threadState %d", threadState);
            return false;
        }
        pthread_attr_t tAttr;

        if (0 != pthread_attr_init(&tAttr))
        {
            CTHREAD_PRINT(startThread, "pthread_attr_init failed on error %s", strerror(errno));
            return false;
        }

        if (0 != pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_JOINABLE))
        {
            CTHREAD_PRINT(startThread, "pthread_attr_setdetachstate failed on error %s", strerror(errno));
            return false;
        }

        startRoutineParamsTemp<T> sParams;
        sParams.pThis = this;
        sParams.params = functionParams;
        sParams.paramsLength = functionParamsLength;
        sParams.virtualFunction = virtualFunction;
        sParams.object = (void*)&object;

        threadState = COMMON_THREAD_INITIATED;

        if (0 != pthread_create(thread, &tAttr, (void * (*)(void*))(CommonThread::startRoutine), &sParams))
        {
            CTHREAD_PRINT(startThread, "pthread_create failed on error %s", strerror(errno));
            threadState = COMMON_THREAD_NOT_INITED;
            return false;
        }

        timespec sTime;
        memset(&sTime, 0, sizeof(timespec));

        sTime.tv_sec = time(NULL);
        sTime.tv_sec += COMMON_THREAD_WAIT_TIMEOUT / 1000;

        if(0 != sem_timedwait(sParams.threadStartedSema, &sTime))
        {
            CTHREAD_PRINT(startThread, "sem_timedwait failed on error %s", strerror(errno));
            pthread_cancel(*thread);
            threadState = COMMON_THREAD_ERROR;
            return false;
        }

        threadState = COMMON_THREAD_STARTED;
        return true;
    }
    bool isStopThreadReceived();
    bool stopThread();
    bool waitForFinish(unsigned timeout_sec);
    commonThreadState getThreadState();
private:
    static void startRoutine(void *param);
    template <class T>
    void CommonThread::startRoutine(void *param)
    {
        CTHREAD_PRINT(startRoutine, "Thread routine started.");
        startRoutineParams *psParams = (startRoutineParams*) param;

        char *threadParams = new char [psParams->paramsLength];
        memset(threadParams, 0, psParams->paramsLength);
        memcpy(threadParams, psParams->params, psParams->paramsLength);
        void *(*function) (void *) = psParams->function;

        printf("\n");
        for(int i = 0; i < 8; i++)
        {
            printf("%2x", threadParams[i]);
        }
        printf("\n");
        sem_post(psParams->threadStartedSema);
        CTHREAD_PRINT(startRoutine, "User function started. threadParams = %p", threadParams);
        function(threadParams);
        CTHREAD_PRINT(startRoutine, "User function finished.threadParams = %p", threadParams);

        delete []threadParams;
        CTHREAD_PRINT(startRoutine, "Thread routine finished.");
    }
};

typedef struct SructStartRoutineParam
{
    CommonThread   *pThis;
    sem_t          *threadStartedSema;
    unsigned        paramsLength;
    void           *params;
    void         *(*function) (void *);
    SructStartRoutineParam()
    {
        threadStartedSema = new sem_t;
        if(0 != sem_init(threadStartedSema, 0, 0))
        {
            CTHREAD_PRINT(SructStartRoutineParam, "sem_init failed on error %s", strerror(errno));
        }
    }
    ~SructStartRoutineParam()
    {
        delete threadStartedSema;
    }
} startRoutineParams;

#pragma pack (pop)

#endif // COMMONTHREAD_H
