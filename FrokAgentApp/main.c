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
    
    InitFaceCommonLib("agent.log");

    sigintAction.sa_flags = SA_SIGINFO;
    sigemptyset(&sigintAction.sa_mask);
    sigintAction.sa_sigaction = sigintHandler;

    if (-1 == sigaction(SIGINT, &sigintAction, NULL))
    {
        TRACE_F("Failed to set custom action on SIGINT on error %s", strerror(errno));
        return -1;
    }

    //InitFrokAgent();

    DeinitFaceCommonLib();

    return 0;
}
