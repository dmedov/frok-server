#include "faceCommonLib.h"

#include <math.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MODULE_NAME     "COMMON_LIB"

frokCommonContext *commonContext = NULL;

BOOL frokLibCommonParseConfigFile(const char *configFile);

BOOL frokLibCommonParseConfigFile(const char *configFile)
{
    if(commonContext == NULL)
    {
        TRACE_F("Not inited");
        return FALSE;
    }
}

FrokResult frokLibCommonInit(const char *configFilePath)
{
    int result = 0;
    int fd;
    pthread_mutexattr_t mAttr;

    if(commonContext != NULL)
    {
        TRACE_W("Already inited");
        return FROK_RESULT_SUCCESS;
    }

    TRACE_N("Creating memory for commonContext");
    commonContext = calloc(1, sizeof(frokCommonContext));
    if(!commonContext)
    {
        TRACE_F("calloc failed on error %s", strerror(errno));
        return FROK_RESULT_MEMORY_FAULT;
    }
    TRACE_N("commonContext created");

    TRACE_N("Set starting time");
    memset(&commonContext->startTime, 0, sizeof(struct timespec));
    if(-1 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &commonContext->startTime))
    {
        TRACE_F("clock_gettime failed on error %s", strerror(errno));
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }
    TRACE_S("Starting time is set");

    TRACE_N("Configuring common mutex");

    if(0 != (result = pthread_mutexattr_init(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_init failed on error %s", strerror(result));
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }
    if(0 != (result = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP)))
    {
        TRACE_F("pthread_mutexattr_settype failed on error %s", strerror(result));
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != (result = pthread_mutex_init(&commonContext->common_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(result));
        pthread_mutexattr_destroy(&mAttr);
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_S("common mutext inited");

    TRACE_N("Configuring trace mutex");

    if(0 != (result = pthread_mutex_init(&commonContext->trace_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(result));
        pthread_mutexattr_destroy(&mAttr);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_S("trace mutex inited");

    if(0 != (result = pthread_mutexattr_destroy(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_destroy failed on error %s", strerror(result));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(configFilePath == NULL)
    {
        configFilePath = FROK_LIB_COMMON_DEFAULT_CONFIG_FILENAME;
    }

    TRACE_N("Parsing config file %s", configFilePath);

    if(FALSE == frokLibCommonParseConfigFile(configFilePath))
    {
        TRACE_F("Parsing config file failed");
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }
    TRACE_S("Config file parsed");

    TRACE_N("Setting unspecified defaults");
    if(commonContext->outputFile == NULL)
    {
        commonContext->outputFile = calloc(strlen(FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME) + 1, 1);
        if(commonContext->outputFile == NULL)
        {
            TRACE_F("calloc failed on error %s", strerror(errno));
            pthread_mutex_destroy(&commonContext->trace_cs);
            pthread_mutex_destroy(&commonContext->common_cs);
            free(commonContext);
            commonContext = NULL;
            return FROK_RESULT_MEMORY_FAULT;
        }
        strcpy(commonContext->outputFile, FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME);
    }

#ifndef TRACE_DEBUG
    TRACE_S("Setting std fd for release");
    TRACE_S("\tstdin is now /dev/null\n\tstdout is now %s\n\tstderr is now %s", commonContext->outputFile, commonContext->outputFile);

    fd = open("/dev/null", O_RDWR);
    if(fd == -1)
    {
        TRACE_F("Failed to open \"/dev/null\" on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    stdin = fd;

    fd = open(commonContext->outputFile, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if(fd == -1)
    {
        TRACE_F("Failed to open \"%s\" on error %s", commonContext->outputFile, strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    stderr = fd;
    stdout = fd;
#endif //TRACE_DEBUG

    TRACE_N("Init succeed");

    return FROK_RESULT_SUCCESS;
}

void frokLibCommonDeinit()
{
    int result = 0;

    if(commonContext == NULL)
    {
        TRACE_W("Already deinited");
        return;
    }

    if(0 != (result = pthread_mutex_destroy(&commonContext->common_cs)))
    {
        TRACE_W("pthread_mutexattr_destroy failed on error %s", strerror(result));
    }

    if(0 != (result = pthread_mutex_destroy(&commonContext->trace_cs)))
    {
        TRACE_W("pthread_mutexattr_destroy failed on error %s", strerror(result));
    }

    free(commonContext->outputFile);
    commonContext->outputFile;

    free(commonContext);
    commonContext = NULL;
}

const char *FrokResultToString(FrokResult res)
{
    switch(res)
    {
        CASE_RET_STR(FROK_RESULT_SUCCESS);
        CASE_RET_STR(FROK_RESULT_CASCADE_ERROR);
        CASE_RET_STR(FROK_RESULT_UNSPECIFIED_ERROR);
        CASE_RET_STR(FROK_RESULT_NOT_A_FACE);
        CASE_RET_STR(FROK_RESULT_INVALID_PARAMETER);
        CASE_RET_STR(FROK_RESULT_OPENCV_ERROR);
        CASE_RET_STR(FROK_RESULT_NO_MODELS);
        CASE_RET_STR(FROK_RESULT_LINUX_ERROR);
        CASE_RET_STR(FROK_RESULT_INVALID_STATE);
        CASE_RET_STR(FROK_RESULT_SOCKET_ERROR);
        CASE_RET_STR(FROK_RESULT_MEMORY_FAULT);
        default:
        {
            return "UNKNOWN_RESULT";
        }
    }
}

double CalculateGamma(double N)
{
    const long double SQRT2PI = 2.5066282746310005024157652848110452530069867406099383;
    int A = 32, K;
    long double Z = (long double)N;
    long double Sc;
    long double F = 1.0;
    long double Ck;
    long double Sum = SQRT2PI;

    Sc = powl((Z + A), (Z + 0.5));
    Sc *= expl(-1.0 * (Z + A));
    Sc /= Z;

    for(K = 1; K < A; K++)
    {
        Z++;
        Ck = powl(A - K, K - 0.5);
        Ck *= expl(A - K);
        Ck /= F;

        Sum += (Ck / Z);

        F *= (-1.0 * K);
    }

    return (double)(Sum * Sc);
}

double CalculateIncompleteGamma(double S, double Z)
{
    int I;
    double Sc = (1.0 / S);
    double Sum = 1.0;
    double Nom = 1.0;
    double Denom = 1.0;

    if(Z < 0.0)
    {
        TRACE_F("Invalid parameter: Z = %lf", Z);
        return -1;
    }

    Sc *= pow(Z, S);
    Sc *= exp(-Z);

    for(I = 0; I < 200; I++)
    {
        Nom *= Z;
        S++;
        Denom *= S;
        Sum += (Nom / Denom);
    }

    return Sum * Sc;
}

double GetPercantByChiSqruare(int Dof, double Cv)
{
    if(Cv < 0 || Dof < 1)
    {
        TRACE_F("Invalid parameter: Dof = %i, Cv = %lf", Dof, Cv);
        return -1;
    }

    double K = ((double)Dof) * 0.5;
    double X = Cv * 0.5;

    if(Dof == 2)
    {
        return exp(-1.0 * X);
    }

    double PValue = CalculateIncompleteGamma(K, X);
    if(PValue < 0)
    {
        TRACE_F("CalculateIncompleteGamma failed");
        return -1;
    }
    if(isnan(PValue) || isinf(PValue) || PValue <= 1e-8)
    {
        return 1e-14;
    }

    PValue /= CalculateGamma(K);

    return (1.0 - PValue);
}
