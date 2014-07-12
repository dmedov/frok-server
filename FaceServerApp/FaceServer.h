#ifndef FACESERVER_H
#define FACESERVER_H

// FaceServer defaults
#define DEFAULT_PHOTO_BASE_PATH     "/home/zda/faces/"
#define DEFAULT_TARGETS_FOLDER_PATH "/home/zda/faces/"
#define DEFAULT_PORT                27015
#define DEFAULT_NUM_OF_AGENTS       8

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"
#include "FaceActivityAgent.h"

#pragma pack(push, 1)
typedef struct FaceRequest
{
    SOCKET      replySocket;
    unsigned    dataLength;
    char       *data;
} FaceRequest;

class FaceServer
{
public:
private:
    FaceActivityAgent  *agents;
    unsigned char       numOfAgents;
    Network            *network;
    char               *photoBasePath;
    char               *targetsFolderPath;
    unsigned short      localPort;

    CommonThread   *threadCallbackListener;
public:
    FaceServer(unsigned char numOfAgents = DEFAULT_NUM_OF_AGENTS, unsigned short localPort = DEFAULT_PORT, char *photoBasePath = DEFAULT_PHOTO_BASE_PATH, char*targetsFolderPath = DEFAULT_TARGETS_FOLDER_PATH);
    ~FaceServer();

    bool StartFaceServer();
    bool StopFaceServer();

private:
    static void CallbackListener(void *pContext);
    static void NetworkCallback(unsigned evt, SOCKET sock, unsigned length, void *param);
};

#pragma pack(pop)

#endif // FACESERVER_H
