#include <stdlib.h>

#include "FrokServer.h"

// Add SHOW_IMAGE define to preprocessor defines in FaceDetectionApp and FaceDetectionLib projects to see resulting image

#define     NET_CMD_RECOGNIZE    "recognize"
#define     NET_CMD_TRAIN        "train"
#define     NET_CMD_GET_FACES    "get_faces"
#define     NET_CMD_SAVE_FACE    "save_face"

void usage()
{
    printf("FaceServerApp <Agent IP address> <Agent port number>");
    return;
}

int main(int argc, char *argv[])
{
    /*std::vector<AgentInfo*> agentsInfo;
    agentsInfo.push_back(new AgentInfo(0, 0));*/
    //FaceServer server(agentsInfo);

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
    server.StopFrokServer();
    /*InitFaceDetectionLib();

    FilePrintMessage(_SUCC("Starting network server with port = %d"), PORT);
    if (NET_SUCCESS != net.StartNetworkServer())
    {
        FilePrintMessage(_FAIL("Failed to start network. For additional info build project with NET_DEBUG_PRINT flag enabled"));
        DeinitFaceDetectionLib();
        return -1;
    }
    FilePrintMessage(_SUCC("Network server started!"));

    char train[] = "{\"cmd\":\"train\", \"ids\":[\"1\"]}\0";    // cut faces and train base
    callback(NET_RECEIVED_REMOTE_DATA, 1, strlen(train), train);

    //char save_face[] = "{\"cmd\":\"save_face\", \"user_id\":\"5\", \"photo_id\":\"1\", \"face_number\":\"0\"}\0";    // cut faces and train base
    //callback(NET_RECEIVED_REMOTE_DATA, 1, strlen(save_face), save_face);

    //char recognize[] = "{\"cmd\":\"recognize\", \"friends\":[\"1\"], \"photo_id\": \"2\"}\0";    // recognize name = 1.jpg
    //callback(NET_RECEIVED_REMOTE_DATA, 1, strlen(recognize), recognize);

    getchar();

    DeinitFaceDetectionLib();

    printf("success\n");*/
    return 0;
}
