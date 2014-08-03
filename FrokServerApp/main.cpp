#include <stdlib.h>
#include "FrokServer.h"

#include "unistd.h"

void usage()
{
    printf("FaceServerApp <Agent IP address> <Agent port number>");
    return;
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        usage();
        return -1;
    }
    AgentInfo *info = new AgentInfo;
    __int32_t srvIPv4[4];
    sscanf(argv[1], "%u.%u.%u.%u", &srvIPv4[0], &srvIPv4[1], &srvIPv4[2], &srvIPv4[3]);

    info->agentIpV4Address = srvIPv4[3] & 0x000000FF;
    info->agentIpV4Address = ((info->agentIpV4Address << 8) & 0x0000FF00) + (srvIPv4[2] & 0x000000FF);
    info->agentIpV4Address = ((info->agentIpV4Address << 8) & 0x00FFFF00) + (srvIPv4[1] & 0x000000FF);
    info->agentIpV4Address = ((info->agentIpV4Address << 8) & 0xFFFFFF00) + (srvIPv4[0] & 0x000000FF);

    info->agentPortNumber = atoi(argv[2]);

    std::vector<AgentInfo*> agentInfoVec;
    agentInfoVec.push_back(info);
    FrokServer server(agentInfoVec, 27015);
    server.StartFrokServer();

    getchar();

    DeinitFaceDetectionLib();
    return 0;
}
