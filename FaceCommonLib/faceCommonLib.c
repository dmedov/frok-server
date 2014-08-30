#include <math.h>
#include <malloc.h>

#include "faceCommonLib.h"

#define MODULE_NAME     "COMMON_LIB"

static pthread_mutex_t file_cs;
struct timespec startTime;
static BOOL init;

FrokResult InitFaceCommonLib(const char *log_name)
{
    int result = 0;
    pthread_mutexattr_t mAttr;

    if(init == TRUE)
    {
        TRACE_W("Already inited");
        return FROK_RESULT_SUCCESS;
    }

    init = TRUE;

    if(0 != (result = pthread_mutexattr_init(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_init failed on error %s", strerror(result));
        init = FALSE;
        return FROK_RESULT_LINUX_ERROR;
    }
    if(0 != (result = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP)))
    {
        TRACE_F("pthread_mutexattr_settype failed on error %s", strerror(result));
        init = FALSE;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != (result = pthread_mutex_init(&file_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(result));
        init = FALSE;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != (result = pthread_mutex_init(&trace_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(result));
        init = FALSE;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != (result = pthread_mutexattr_destroy(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_destroy failed on error %s", strerror(result));
        init = FALSE;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(log_name != NULL)
    {
        pthread_mutex_lock(&file_cs);

        free(log_file);
        log_file = malloc(strlen(log_name) + 1);
        if(!log_file)
        {
            TRACE_F("malloc failed on error %s", strerror(errno));
            init = FALSE;
            return FROK_RESULT_MEMORY_FAULT;
        }

        strcpy(log_file, log_name);

        pthread_mutex_unlock(&file_cs);
    }

    memset(&startTime, 0, sizeof(startTime));
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);

    return FROK_RESULT_SUCCESS;
}

FrokResult DeinitFaceCommonLib()
{
    int result = 0;

    if(init == FALSE)
    {
        TRACE_W("Already deinited");
        return FROK_RESULT_SUCCESS;
    }

    init = FALSE;

    free(log_file);

    if(0 != (result = pthread_mutex_destroy(&file_cs)))
    {
        TRACE_F("pthread_mutexattr_destroy failed on error %s", strerror(result));
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != (result = pthread_mutex_destroy(&trace_cs)))
    {
        TRACE_F("pthread_mutexattr_destroy failed on error %s", strerror(result));
        return FROK_RESULT_LINUX_ERROR;
    }

    return FROK_RESULT_SUCCESS;
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
