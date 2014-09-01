#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "FrokServer.h"



#define MODULE_NAME "SERVER"

//FrokServer *server;

void usage()
{
    printf("FaceServerApp <Agent IP address> <Agent port number>");
    return;
}

static void sigintHandler(int UNUSED(sig), siginfo_t *UNUSED(si), void *UNUSED(p))
{
    TRACE_S("SIGINT captured!");
    /*if(server != NULL)
    {
        //server->StopFrokServer();
    }*/
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        usage();
        return -1;
    }
   /* AgentInfo *info = new AgentInfo;
    __int32_t srvIPv4[4];
    sscanf(argv[1], "%u.%u.%u.%u", &srvIPv4[0], &srvIPv4[1], &srvIPv4[2], &srvIPv4[3]);

    info->agentIpV4Address = srvIPv4[3] & 0x000000FF;
    info->agentIpV4Address = ((info->agentIpV4Address << 8) & 0x0000FF00) + (srvIPv4[2] & 0x000000FF);
    info->agentIpV4Address = ((info->agentIpV4Address << 8) & 0x00FFFF00) + (srvIPv4[1] & 0x000000FF);
    info->agentIpV4Address = ((info->agentIpV4Address << 8) & 0xFFFFFF00) + (srvIPv4[0] & 0x000000FF);

    info->agentPortNumber = atoi(argv[2]);

    std::vector<AgentInfo*> agentInfoVec;
    agentInfoVec.push_back(info);

    server = new FrokServer(agentInfoVec, 27015);

    struct sigaction sigintAction;

   sigintAction.sa_flags = SA_SIGINFO;
   sigemptyset(&sigintAction.sa_mask);
   sigintAction.sa_sigaction = sigintHandler;

   if (-1 == sigaction(SIGINT, &sigintAction, NULL))
   {
       TRACE_F("Failed to set custom action on SIGINT on error %s", strerror(errno));
       return -1;
   }

    server->StartFrokServer();

    TRACE("Finished");*/
    return 0;
}
