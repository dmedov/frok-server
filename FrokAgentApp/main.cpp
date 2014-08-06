#include <stdio.h>
#include <signal.h>
#include "FrokAgent.h"

#define MODULE_NAME "AGENT"

FrokAgent *agent;

void usage()
{
    printf("FaceDetectionApp <Local port number>");
    return;
}

static void sigusr1Handler(int sig, siginfo_t *si, void *p)
{
    UNREFERENCED_PARAMETER(sig);
    UNREFERENCED_PARAMETER(si);
    UNREFERENCED_PARAMETER(p);

    TRACE_S("SIGUSR1 captured!");
    if(agent != NULL)
    {
        agent->StopFrokAgent();
    }
}

int main(void)
{
    std::map <std::string, FrokAPIFunction*> functions;
    functions["train"] = &FAPI_TrainUserModel;
    functions["recognize"] = &FAPI_Recognize;

    agent = new FrokAgent(functions);

    struct sigaction sigusr1Action;

   sigusr1Action.sa_flags = SA_SIGINFO;
   sigemptyset(&sigusr1Action.sa_mask);
   sigusr1Action.sa_sigaction = sigusr1Handler;

   if (-1 == sigaction(SIGUSR1, &sigusr1Action, NULL))
   {
       TRACE_F("Failed to set custom action on SIGUSR1 on error %s", strerror(errno));
       return -1;
   }

    agent->StartFrokAgent();

    delete agent;

    TRACE("Finished");
    return 0;
}
