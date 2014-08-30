#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include "FrokAgent.h"

#define MODULE_NAME "AGENT"



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
    
    if(FROK_RESULT_SUCCESS != (res = InitFaceCommonLib("agent.log")))
    {
        TRACE("InitFaceCommonLib failed on error %s", FrokResultToString(res));
        return -1;
    }

    sigintAction.sa_flags = SA_SIGINFO;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_sigaction = sigintHandler;

    TRACE_N("Setting custom SIGINT action...");
    if (-1 == sigaction(SIGINT, &sigintAction, NULL))
    {
        TRACE_F("Failed to set custom action on SIGINT on error %s", strerror(errno));
        DeinitFaceCommonLib();
        return -1;
    }
    TRACE_S("sigaction succeed");

    TRACE_N("Calling frokAgentInit...");
    if(FROK_RESULT_SUCCESS != (res = frokAgentInit(27015, DEFAULT_PHOTO_BASE_PATH, DEFAULT_TARGETS_FOLDER_PATH)))
    {
        TRACE_F("frokAgentInit failed on error %s", FrokResultToString(res));
        DeinitFaceCommonLib();
        return -1;
    }
    TRACE_S("frokAgentInit succeed");

    TRACE_N("Calling frokAgentStart...");
    // Inifinite loop from this function. To stop application send SIGINT signal (^C from console)
    if(FROK_RESULT_SUCCESS != (res = frokAgentStart()))
    {
        TRACE_F("frokAgentStart failed on error %s", FrokResultToString(res));
        frokAgentDeinit();
        DeinitFaceCommonLib();
        return -1;
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
        DeinitFaceCommonLib();
        return -1;
    }
    TRACE_S("sigaction succeed");

    TRACE_N("Calling frokAgentDeinit...");
    if(FROK_RESULT_SUCCESS != (res = frokAgentDeinit()))
    {
        TRACE_F("frokAgentDeinit failed on error %s", FrokResultToString(res));
        DeinitFaceCommonLib();
        return -1;
    }
    TRACE_S("frokAgentDeinit succeed");

    if(FROK_RESULT_SUCCESS != (res = DeinitFaceCommonLib()))
    {
        TRACE("InitFaceCommonLib failed on error %s", FrokResultToString(res));
        return -1;
    }
    return 0;
}
