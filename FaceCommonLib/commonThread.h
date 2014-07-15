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
    bool isStopThreadReceived();
    bool stopThread();
    bool waitForFinish(unsigned timeout_sec);
    commonThreadState getThreadState();
private:
    static void startRoutine(void *param);
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
