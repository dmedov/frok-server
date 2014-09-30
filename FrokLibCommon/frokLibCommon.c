#include "frokLibCommon.h"

#include <math.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define MODULE_NAME     "COMMON_LIB"

char *tracePrefix = NULL;
frokCommonContext *commonContext = NULL;

BOOL frokLibCommonParseConfigFile(const char *configFile);

BOOL frokLibCommonParseConfigFile(const char *configFile)
{
    char *configFilePath;
    FILE *configStream;
    int configDescriptor;
    char lineBuf[LINE_MAX];
    char tmp;
    int i, j;
    BOOL noWritting = FALSE;

    if(commonContext == NULL)
    {
        TRACE_F("Not inited");
        return FALSE;
    }

    // We don't need to create the "/" directory, so we don't need i >= 0 in this cycle
    for(i = strlen(configFile); i > 0; i--)
    {
        if(configFile[i] == '/')
        {
            break;
        }
    }

    if(i != 0)
    {
        configFilePath = calloc(i + 1, 1);
        if(!configFilePath)
        {
            TRACE_F("calloc failed on error %s", strerror(errno));
            return FALSE;
        }

        strncpy(configFilePath, configFile, i);

        TRACE_N("Making required directories");
        if(FALSE == mkpath(configFilePath, 0775))
        {
            TRACE_F("Failed to make required directories");
            return FALSE;
        }

        free(configFilePath);
        configFilePath = NULL;
    }

    TRACE_N("Opening config file. File is automatically created if it doesn't exist");
    configDescriptor = open(configFile, O_CREAT | O_RDWR, 0664);
    if(configDescriptor == -1)
    {
        if(errno == EACCES)
        {
            configDescriptor = open(configFile, O_RDONLY);
            if(configDescriptor == -1)
            {
                TRACE_F("failed to open config file on error %s", strerror(errno));
                return FALSE;
            }
            noWritting = TRUE;
        }
        else
        {
            TRACE_F("failed to open config file on error %s", strerror(errno));
            return FALSE;
        }
    }

    TRACE_N("Opening config file stream (fd = %x)", configDescriptor);
    if(noWritting == FALSE)
    {
        configStream = fdopen(configDescriptor, "r+");
    }
    else
    {
        configStream = fdopen(configDescriptor, "r");
    }
    if(configStream == NULL)
    {
        TRACE_F("Failed to open config file stream on error %s", strerror(errno));
        close(configDescriptor);
        return FALSE;
    }

    TRACE_S("Config file stream opened");

    free(commonContext->outputFile);
    free(commonContext->photoBasePath);
    free(commonContext->targetPhotosPath);

    commonContext->outputFile = NULL;
    commonContext->photoBasePath = NULL;
    commonContext->targetPhotosPath = NULL;

    while(NULL != fgets(lineBuf, LINE_MAX, configStream))
    {
        if(0 == strncmp(lineBuf, FROK_PARAM_OUTPUT_FILENAME, strlen(FROK_PARAM_OUTPUT_FILENAME)))
        {
            // Find start position of parameter
            for(i = strlen(FROK_PARAM_OUTPUT_FILENAME); i < strlen(lineBuf); i++)
            {
                tmp = lineBuf[i];
                if((tmp == '\r') || (tmp == '\n') || (tmp == '=') || (tmp == ' '))
                {
                    continue;
                }
                break;
            }
            if(i == strlen(lineBuf))
            {
                TRACE_N("Parameter is empty");
                continue;
            }
            // find last position of parameter
            for(j = strlen(lineBuf) - 1; j >= i; j--)
            {
                tmp = lineBuf[j];
                if((tmp == '\r') || (tmp == '\n'))
                {
                    continue;
                }

                j++;
                break;
            }

            if(i > j)
            {
                TRACE_N("Parameter is surely empty");
                continue;
            }

            free(commonContext->outputFile);
            commonContext->outputFile = NULL;
            commonContext->outputFile = calloc(j - i + 1, 1);
            if(commonContext->outputFile == NULL)
            {
                TRACE_F("calloc failed on error %s", strerror(errno));
                fclose(configStream);
                return FALSE;
            }

            strncpy(commonContext->outputFile, lineBuf + i, j - i);
            TRACE_S("Output file is %s", commonContext->outputFile);
        }
        else if(0 == strncmp(lineBuf, FROK_PARAM_PHOTO_BASE_PATH, strlen(FROK_PARAM_PHOTO_BASE_PATH)))
        {
            // Find start position of parameter
            for(i = strlen(FROK_PARAM_PHOTO_BASE_PATH); i < strlen(lineBuf); i++)
            {
                tmp = lineBuf[i];
                if((tmp == '\r') || (tmp == '\n') || (tmp == '=') || (tmp == ' '))
                {
                    continue;
                }
                break;
            }
            if(i == strlen(lineBuf))
            {
                TRACE_N("Parameter is empty");
                continue;
            }
            // find last position of parameter
            for(j = strlen(lineBuf) - 1; j >= i; j--)
            {
                tmp = lineBuf[j];
                if((tmp == '\r') || (tmp == '\n'))
                {
                    continue;
                }

                j++;
                break;
            }

            if(i > j)
            {
                TRACE_N("Parameter is surely empty");
                continue;
            }

            free(commonContext->photoBasePath);
            commonContext->photoBasePath = NULL;
            commonContext->photoBasePath = calloc(j - i + 1, 1);
            if(commonContext->photoBasePath == NULL)
            {
                TRACE_F("calloc failed on error %s", strerror(errno));
                fclose(configStream);
                return FALSE;
            }

            strncpy(commonContext->photoBasePath, lineBuf + i, j - i);
            TRACE_S("PhotoBasePath is %s", commonContext->photoBasePath);
            continue;
        }
        else if(0 == strncmp(lineBuf, FROK_PARAM_TARGET_PHOTOS_PATH, strlen(FROK_PARAM_TARGET_PHOTOS_PATH)))
        {
            // Find start position of parameter
            for(i = strlen(FROK_PARAM_TARGET_PHOTOS_PATH); i < strlen(lineBuf); i++)
            {
                tmp = lineBuf[i];
                if((tmp == '\r') || (tmp == '\n') || (tmp == '=') || (tmp == ' '))
                {
                    continue;
                }
                break;
            }
            if(i == strlen(lineBuf))
            {
                TRACE_N("Parameter is empty");
                continue;
            }
            // find last position of parameter
            for(j = strlen(lineBuf) - 1; j >= i; j--)
            {
                tmp = lineBuf[j];
                if((tmp == '\r') || (tmp == '\n'))
                {
                    continue;
                }

                j++;
                break;
            }

            if(i > j)
            {
                TRACE_N("Parameter is surely empty");
                continue;
            }

            free(commonContext->targetPhotosPath);
            commonContext->targetPhotosPath = NULL;
            commonContext->targetPhotosPath = calloc(j - i + 1, 1);
            if(commonContext->targetPhotosPath== NULL)
            {
                TRACE_F("calloc failed on error %s", strerror(errno));
                fclose(configStream);
                return FALSE;
            }

            strncpy(commonContext->targetPhotosPath, lineBuf + i, j - i);
            TRACE_S("TargetPhotosPath is %s", commonContext->targetPhotosPath);
            continue;
        }
    }

    // Fill frok.conf with default values if they are not specified

    if(commonContext->outputFile == NULL || commonContext->photoBasePath == NULL || commonContext->targetPhotosPath == NULL)
    {
        TRACE_N("Saving default values of unset parameters to config file");

        if(commonContext->outputFile == NULL)
        {
            if(noWritting == FALSE)
            {
                strcpy(lineBuf, FROK_PARAM_OUTPUT_FILENAME);
                strcat(lineBuf + strlen(FROK_PARAM_OUTPUT_FILENAME), " = ");
                strcat(lineBuf + strlen(FROK_PARAM_OUTPUT_FILENAME) + strlen(" = "), FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME);
                strcat(lineBuf + strlen(FROK_PARAM_OUTPUT_FILENAME) + strlen(" = ") + strlen(FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME), "\n\0");
                if(EOF == fputs(lineBuf, configStream))
                {
                    TRACE_F("fputs failed");
                    fclose(configStream);
                    return FALSE;
                }
            }

            commonContext->outputFile = calloc(strlen(FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME) + 1, 1);
            if(!commonContext->outputFile)
            {
                TRACE_F("calloc failed on error %s", strerror(errno));
                fclose(configStream);
                return FALSE;
            }
            strcpy(commonContext->outputFile, FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME);

            TRACE_S("output file is %s", FROK_LIB_COMMON_DEFAULT_OUTPUT_FILENAME);
        }

        if(commonContext->photoBasePath == NULL)
        {
            if(noWritting == FALSE)
            {
                strcpy(lineBuf, FROK_PARAM_PHOTO_BASE_PATH);
                strcat(lineBuf + strlen(FROK_PARAM_PHOTO_BASE_PATH), " = ");
                strcat(lineBuf + strlen(FROK_PARAM_PHOTO_BASE_PATH) + strlen(" = "), FROK_DEFAULT_PHOTO_BASE_PATH);
                strcat(lineBuf + strlen(FROK_PARAM_PHOTO_BASE_PATH) + strlen(" = ") + strlen(FROK_DEFAULT_PHOTO_BASE_PATH), "\n\0");
                if(EOF == fputs(lineBuf, configStream))
                {
                    TRACE_F("fputs failed");
                    fclose(configStream);
                    return FALSE;
                }
            }

            commonContext->photoBasePath = calloc(strlen(FROK_DEFAULT_PHOTO_BASE_PATH) + 1, 1);
            if(!commonContext->photoBasePath)
            {
                TRACE_F("calloc failed on error %s", strerror(errno));
                fclose(configStream);
                return FALSE;
            }
            strcpy(commonContext->photoBasePath, FROK_DEFAULT_PHOTO_BASE_PATH);

            TRACE_S("photoBasePath is %s", FROK_DEFAULT_PHOTO_BASE_PATH);
        }

        if(commonContext->targetPhotosPath == NULL)
        {
            if(noWritting == FALSE)
            {
                strcpy(lineBuf, FROK_PARAM_TARGET_PHOTOS_PATH);
                strcat(lineBuf + strlen(FROK_PARAM_TARGET_PHOTOS_PATH), " = ");
                strcat(lineBuf + strlen(FROK_PARAM_TARGET_PHOTOS_PATH) + strlen(" = "), FROK_DEFAULT_TARGET_PHOTOS_PATH);
                strcat(lineBuf + strlen(FROK_PARAM_TARGET_PHOTOS_PATH) + strlen(" = ") + strlen(FROK_DEFAULT_TARGET_PHOTOS_PATH), "\n\0");
                if(EOF == fputs(lineBuf, configStream))
                {
                    TRACE_F("fputs failed");
                    fclose(configStream);
                    return FALSE;
                }
            }

            commonContext->targetPhotosPath = calloc(strlen(FROK_DEFAULT_TARGET_PHOTOS_PATH) + 1, 1);
            if(!commonContext->targetPhotosPath)
            {
                TRACE_F("calloc failed on error %s", strerror(errno));
                fclose(configStream);
                return FALSE;
            }
            strcpy(commonContext->targetPhotosPath, FROK_DEFAULT_TARGET_PHOTOS_PATH);

            TRACE_S("targetPhotosPath is %s", FROK_DEFAULT_TARGET_PHOTOS_PATH);
        }
    }

    if(EOF == fclose(configStream))
    {
        TRACE_F("fclose failed on error %s", strerror(errno));
        return FALSE;
    }

    TRACE_S("Parsing finished");
    return TRUE;
}

void set_trace_prefix(const char *prefix)
{
    if(prefix != NULL)
    {

        free(tracePrefix);
        tracePrefix = NULL;
        tracePrefix = calloc(strlen(prefix) + 1, 1);
        if(tracePrefix != NULL)
        {
            strcpy(tracePrefix, prefix);
        }
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
    TRACE_S("See output logs in file %s", commonContext->outputFile);

    fd = open("/dev/null", O_RDWR);
    if(fd == -1)
    {
        TRACE_F("Failed to open \"/dev/null\" on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != fclose(stdin))
    {
        TRACE_F("Failed to close stdin on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    stdin = fdopen(fd, "r");
    if(!stdin)
    {
        TRACE_F("fdopen failed on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    fd = open(commonContext->outputFile, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if(fd == -1)
    {
        TRACE_F("Failed to open \"%s\" on error %s", commonContext->outputFile, strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != fclose(stdout))
    {
        TRACE_F("Failed to close stdout on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    stdout = fdopen(fd, "w");
    if(!stdout)
    {
        TRACE_F("fdopen failed on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    if(0 != fclose(stderr))
    {
        TRACE_F("Failed to close stderr on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    stderr = fdopen(fd, "w");
    if(!stderr)
    {
        TRACE_F("fdopen failed on error %s", strerror(errno));
        pthread_mutex_destroy(&commonContext->trace_cs);
        pthread_mutex_destroy(&commonContext->common_cs);
        free(commonContext->outputFile);
        commonContext->outputFile = NULL;
        free(commonContext);
        commonContext = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }
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

    free(commonContext->photoBasePath);
    free(commonContext->targetPhotosPath);
    free(commonContext->outputFile);
    commonContext->outputFile = NULL;
    commonContext->targetPhotosPath = NULL;
    commonContext->photoBasePath = NULL;

    free(commonContext);
    commonContext = NULL;
}

const char *FrokResultToString(FrokResult res)
{
    switch(res)
    {
        CASE_RET_STR(FROK_RESULT_SUCCESS);
        CASE_RET_STR(FROK_RESULT_UNSPECIFIED_ERROR);
        CASE_RET_STR(FROK_RESULT_NO_FACES_FOUND);
        CASE_RET_STR(FROK_RESULT_INVALID_PARAMETER);
        CASE_RET_STR(FROK_RESULT_OPENCV_ERROR);
        CASE_RET_STR(FROK_RESULT_NO_MODELS);
        CASE_RET_STR(FROK_RESULT_LINUX_ERROR);
        CASE_RET_STR(FROK_RESULT_INVALID_STATE);
        CASE_RET_STR(FROK_RESULT_SOCKET_ERROR);
        CASE_RET_STR(FROK_RESULT_MEMORY_FAULT);
        CASE_RET_STR(FROK_RESULT_PERM_ERROR);
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
