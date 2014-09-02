#include "FrokAgent.h"

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>

#define MODULE_NAME "AGENT"

//{"cmd":"train", "arrIds":["1"]}
//{"cmd":"recognize", "arrIds":["1"], "photoName":"1.jpg"}

static void sigintHandler(int UNUSED(sig), siginfo_t UNUSED(*si), void UNUSED(*p))
{
    if(FROK_RESULT_SUCCESS != frokAgentStop())
    {
        if(FROK_RESULT_SUCCESS != frokAgentStop())
        {
            // Something went wrong - shutdown
            frokAgentDeinit();
        }
    }
}

void usage()
{
    TRACE("FrokAgentApp <CPU number> <Agent's port number>\n");
    TRACE("If you set CPU number to -1 than Process will spawn N daemons, where N is number of available CPUs.\n");
    TRACE("In this case Agent's port number will be the number of first one. Port number of 2nd one = port number of 1st + 1, etc\n");
}

int main(int argc, char *argv[])
{
    struct sigaction sigintAction;
    FrokResult res;
    short cpu_number;
    unsigned short port_number;
    pid_t pid;
    cpu_set_t set;
    unsigned short i;
    char **newProcessArguments;

    if(argc != 3)
    {
        usage();
        exit(EXIT_FAILURE);
    }

    cpu_number = (short)atoi(argv[1]);
    port_number = (unsigned short)atoi(argv[2]);

    if(cpu_number == -1)
    {
        TRACE_S("Start spawning agents");
        if(-1 == sched_getaffinity(0, sizeof(cpu_set_t), &set))
        {
            TRACE_F("sched_getaffinity failed on error %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        for(i = 0; i < CPU_SETSIZE; i++)
        {
            if(CPU_ISSET(i, &set))
            {
                pid = fork();
                if(-1 == pid)
                {
                    TRACE_F("fork failed on error %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(0 == pid)
                {
                    newProcessArguments = malloc(sizeof(char *) * 4);
                    if(!newProcessArguments)
                    {
                        TRACE_F("malloc failed on error %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    // Binary name
                    newProcessArguments[0] = calloc(strlen(argv[0]) + 1, 1);
                    if(!newProcessArguments[0])
                    {
                        TRACE_F("calloc failed on error %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    strcpy(newProcessArguments[0], argv[0]);

                    // CPU number
                    newProcessArguments[1] = calloc(4 + 1, 1);
                    if(!newProcessArguments[1])
                    {
                        TRACE_F("calloc failed on error %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    sprintf(newProcessArguments[1], "%d", i);

                    // Port number
                    newProcessArguments[2] = calloc(4 + 1, 1);
                    if(!newProcessArguments[2])
                    {
                        TRACE_F("calloc failed on error %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    sprintf(newProcessArguments[2], "%d", port_number);

                    newProcessArguments[3] = NULL;

                    TRACE_N("Running %s %s %s", newProcessArguments[0], newProcessArguments[1], newProcessArguments[2]);
                    fflush(stdout);
                    if(-1 == execve(newProcessArguments[0], newProcessArguments, NULL))
                    {
                        TRACE_F("execve failed on error %s", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    port_number++;
                }
            }
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        set_trace_prefix(argv[1]);

        TRACE_N("Obtaining CPU #%d", cpu_number);
        if(FALSE == obtainCPU(cpu_number))
        {
            TRACE_F("failed to obtaingCPU #%d", cpu_number);
            exit(EXIT_FAILURE);
        }
        TRACE_N("CPU obtained");
    }

    if(FROK_RESULT_SUCCESS != (res = frokLibCommonInit(NULL)))
    {
        TRACE_N("frokLibCommonInit failed on error %s", FrokResultToString(res));
        exit(EXIT_FAILURE);
    }

#ifndef NO_DAEMON
    TRACE_N("Becoming a daemon");
    if(FALSE == frokBecomeADaemon())
    {
        TRACE_F("frokBecomeADaemon failed");
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("Agent is daemon now");
#endif
    if(FALSE == obtainCPU(cpu_number))
    {
        TRACE_F("obtainCPU failed");
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }

    sigintAction.sa_flags = SA_SIGINFO;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_sigaction = sigintHandler;

    TRACE_N("Setting custom SIGINT action...");
    if (-1 == sigaction(SIGINT, &sigintAction, NULL))
    {
        TRACE_F("Failed to set custom action on SIGINT on error %s", strerror(errno));
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("sigaction succeed");

    TRACE_N("Create detector instance");
    void *detector = frokFaceDetectorAlloc();
    if(detector == NULL)
    {
        TRACE_F("Failed to create detector instance");
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("detector instance created");


    TRACE_N("Create recognizer instance");
    void *recognizer = frokFaceRecognizerEigenfacesAlloc();
    if(recognizer == NULL)
    {
        TRACE_F("Failed to create recognizer instance");
        frokFaceDetectorDealloc(detector);
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("recognizer instance created");

    TRACE_N("Init model with users from path %s", DEFAULT_PHOTO_BASE_PATH);
    if(FALSE == frokFaceRecognizerEigenfacesInit(recognizer, DEFAULT_PHOTO_BASE_PATH))
    {
        TRACE_F("Failed to init recognizer base. Continue");
    }
    else
    {
        TRACE_S("Recognize base inited");
    }

    TRACE_N("Calling frokAgentInit...");
    if(FROK_RESULT_SUCCESS != (res = frokAgentInit(port_number, detector, recognizer, DEFAULT_PHOTO_BASE_PATH, DEFAULT_TARGETS_FOLDER_PATH)))
    {
        TRACE_F("frokAgentInit failed on error %s", FrokResultToString(res));
        frokFaceDetectorDealloc(detector);
        frokFaceRecognizerEigenfacesDealloc(recognizer);
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("frokAgentInit succeed");

    TRACE_N("Calling frokAgentStart...");
    // Inifinite loop from this function. To stop application send SIGINT signal (^C from console)
    if(FROK_RESULT_SUCCESS != (res = frokAgentStart()))
    {
        TRACE_F("frokAgentStart failed on error %s", FrokResultToString(res));
        frokAgentDeinit();
        frokFaceDetectorDealloc(detector);
        frokFaceRecognizerEigenfacesDealloc(recognizer);
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("frokAgentStart succeed");

    // SIGINT captured - close application, set default handlers, etc...

    sigintAction.sa_flags = 0;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_handler = SIG_DFL;

    TRACE_N("Setting default SIGINT action...");
    if (-1 == sigaction(SIGINT, &sigintAction, NULL))
    {
        TRACE_F("Failed to set custom action on SIGINT on error %s", strerror(errno));
        frokAgentDeinit();
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("sigaction succeed");

    TRACE_N("Calling frokAgentDeinit...");
    if(FROK_RESULT_SUCCESS != (res = frokAgentDeinit()))
    {
        TRACE_F("frokAgentDeinit failed on error %s", FrokResultToString(res));
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("frokAgentDeinit succeed");

    TRACE_N("Delete detector instance");
    frokFaceDetectorDealloc(detector);
    TRACE_S("Detector instance deleted");

    TRACE_N("Delete recognizer instance");
    frokFaceRecognizerEigenfacesDealloc(recognizer);
    TRACE_S("Recognizer instance deleted");

    TRACE_N("Deinig lib common");
    frokLibCommonDeinit();

    exit(EXIT_SUCCESS);
}
