#ifndef FROKAGENT_H
#define FROKAGENT_H

#define DEFAULT_PORT                28015

// include dependencies
#include "frokLibCommon.h"
#include "FrokAPI.h"
#include "pthread.h"

// Client data timeout. If timeout reqached and no data received - disconnect
#define FROK_AGENT_CLIENT_DATA_TIMEOUT_MS       10000

#pragma pack(push, 1)
typedef struct FrokAgentContext
{
    SOCKET localSock;
    unsigned short localPortNumber;
    BOOL agentStarted;
    pthread_t agentThread;
    int terminateAgentEvent;
    void *api;
}FrokAgentContext;
#pragma pack(pop)

FrokResult frokAgentInit(unsigned short port, void *detector, void *recognizer, const char *photoBaseFolderPath, const char *targetsFolderPath);
FrokResult frokAgentStart();
FrokResult frokAgentStop();
FrokResult frokAgentDeinit();

#endif // FROKAGENT_H
