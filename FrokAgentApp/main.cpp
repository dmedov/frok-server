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

static void sigintHandler(int sig, siginfo_t *si, void *p)
{
    UNREFERENCED_PARAMETER(sig);
    UNREFERENCED_PARAMETER(si);
    UNREFERENCED_PARAMETER(p);

    TRACE_S("SIGINT captured!");
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

    struct sigaction sigintAction;

   sigintAction.sa_flags = SA_SIGINFO;
   sigemptyset(&sigintAction.sa_mask);
   sigintAction.sa_sigaction = sigintHandler;

   if (-1 == sigaction(SIGINT, &sigintAction, NULL))
   {
       TRACE_F("Failed to set custom action on SIGINT on error %s", strerror(errno));
       return -1;
   }

    agent->StartFrokAgent();

    delete agent;

    TRACE("Finished");
    return 0;
}
