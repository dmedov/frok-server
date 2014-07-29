#include <pthread.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include "faceCommonLib.h"

#define MODULE_NAME     "COMMON_LIB"

pthread_mutex_t filePrint_cs;
struct timeval startTime;

bool InitFaceCommonLib(const char *log_name)
{
    int result = 0;

    pthread_mutexattr_t mAttr;
    if(0 != (result = pthread_mutexattr_init(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_init failed on error %s", strerror(result));
        return false;
    }
    if(0 != (result = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP)))
    {
        TRACE_F("pthread_mutexattr_settype failed on error %s", strerror(result));
        return false;
    }

    if(0 != (result = pthread_mutex_init(&filePrint_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(result));
        return false;
    }

    if(0 != (result = pthread_mutexattr_destroy(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_destroy failed on error %s", strerror(result));
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

    memset(&startTime, 0, sizeof(startTime));
    gettimeofday(&startTime, NULL);

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

void set_time_stamp(unsigned *sec, unsigned *usec)
{
    struct timeval currentTime;
    memset(&currentTime, 0, sizeof(currentTime));

    gettimeofday(&currentTime, NULL);

    (*sec)  = currentTime.tv_sec - startTime.tv_sec;
    (*usec) = currentTime.tv_usec - startTime.tv_usec;

    if (startTime.tv_usec > currentTime.tv_usec)
    {
        (*sec)--;
        (*usec) += 1e6;
    }
}

void print_time(timespec &startTime, timespec &endTime)
{
    unsigned sec  = endTime.tv_sec - startTime.tv_sec;
    unsigned nsec = endTime.tv_nsec - startTime.tv_nsec;

    if (startTime.tv_nsec > endTime.tv_nsec)
    {
        sec--;
        nsec += 1e9;
    }

    fprintf(stdout, "Time elapsed: %u.%09u\n", sec, nsec);
}

char *FrokResultToString(FrokResult res)
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
        default:
        {
            return "UNKNOWN_RESULT";
        }
    }
}

double CalculateGamma(double N)
{
    const long double SQRT2PI = 2.5066282746310005024157652848110452530069867406099383;

    int A = 32;
    long double Z = (long double)N;
    long double Sc = powl((Z + A), (Z + 0.5));
    Sc *= expl(-1.0 * (Z + A));
    Sc /= Z;

    long double F = 1.0;
    long double Ck;
    long double Sum = SQRT2PI;


    for(int K = 1; K < A; K++)
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
    if(Z < 0.0)
    {
        TRACE_F("Invalid parameter: Z = %lf", Z);
        return -1;
    }
    double Sc = (1.0 / S);
    Sc *= pow(Z, S);
    Sc *= exp(-Z);

    double Sum = 1.0;
    double Nom = 1.0;
    double Denom = 1.0;

    for(int I = 0; I < 200; I++)
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
