#ifndef COMMONTHREAD_H
#define COMMONTHREAD_H

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

class CommonThread
{
private:
    commonThreadState   threadState;
    pthread_t           thread;
    sem_t               threadStoppedSema;

public:
    CommonThread();
    ~CommonThread();

    bool startThread(void *(*function) (void *), void *functionParams, unsigned functionParamsLength);
    bool isStopThreadReceived();
    bool stopThread();
    commonThreadState getThreadState();
private:
    static void startRoutine(void *param);
};

typedef struct SructStartRoutineParam
{
    CommonThread   *pThis;
    sem_t           threadStartedSema;
    unsigned        paramsLength;
    void           *params;
    void         *(*function) (void *);
} startRoutineParams;

#pragma pack (pop)

#endif // COMMONTHREAD_H
