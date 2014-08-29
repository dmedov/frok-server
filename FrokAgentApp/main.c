#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sched.h>
#include "FrokAgent.h"

#define MODULE_NAME "AGENT"



static void sigintHandler(int UNUSED(sig), siginfo_t UNUSED(*si), void UNUSED(*p))
{
    TRACE_S("SIGINT captured!");
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
    
    InitFaceCommonLib("agent.log");

    sigintAction.sa_flags = SA_SIGINFO;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_sigaction = sigintHandler;

    if (-1 == sigaction(SIGINT, &sigintAction, NULL))
    {
        TRACE_F("Failed to set custom action on SIGINT on error %s", strerror(errno));
        return -1;
    }

    while(1)
    {
        if(FROK_RESULT_SUCCESS != (res = frokAgentInit(1, NULL, NULL)))
        {
            TRACE_F("frokAgentInit failed on error %s", FrokResultToString(res));
            return -1;
        }

        if(FROK_RESULT_SUCCESS != (res = frokAgentStart()))
        {
            TRACE_F("frokAgentStart failed on error %s", FrokResultToString(res));
            return -1;
        }

        if(FROK_RESULT_SUCCESS != (res = frokAgentStop()))
        {
            TRACE_F("frokAgentStop failed on error %s", FrokResultToString(res));
            return -1;
        }

        if(FROK_RESULT_SUCCESS != (res = frokAgentDeinit()))
        {
            TRACE_F("frokAgentDeinit failed on error %s", FrokResultToString(res));
            return -1;
        }
    }

    //InitFrokAgent();

    DeinitFaceCommonLib();

    return 0;
}
