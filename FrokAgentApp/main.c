#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include "FrokAgent.h"

#define MODULE_NAME "AGENT"

//{"cmd":"train", "arrIds":["1"]}
//{"cmd":"recognize", "arrIds":["1"], "photoName":"1"}

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
    printf("FaceServerApp <Agent IP address> <Agent port number>");
    return;
}

int main(int argc, char *argv[])
{
    struct sigaction sigintAction;
    FrokResult res;
    
    if(FROK_RESULT_SUCCESS != (res = frokLibCommonInit(NULL)))
    {
        TRACE_N("frokLibCommonInit failed on error %s", FrokResultToString(res));
        exit(EXIT_FAILURE);
    }

    TRACE_N("Becoming a daemon");
    if(FALSE == frokBecomeADaemon())
    {
        TRACE_F("frokBecomeADaemon failed");
        frokLibCommonDeinit();
        exit(EXIT_FAILURE);
    }
    TRACE_S("Agent is daemon now");

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

    TRACE_N("Calling frokAgentInit...");
    if(FROK_RESULT_SUCCESS != (res = frokAgentInit(27015, detector, recognizer, DEFAULT_PHOTO_BASE_PATH, DEFAULT_TARGETS_FOLDER_PATH)))
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
