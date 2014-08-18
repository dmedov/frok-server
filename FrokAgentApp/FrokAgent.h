#ifndef FROKAGENT_H
#define FROKAGENT_H

#define DEFAULT_PORT                28015

// include dependencies
#include "faceCommonLib.h"

typedef struct FrokAgentContext
{
    SOCKET localSock;
}FrokAgentContext;

BOOL InitFrokAgent(FrokAgentContext context, unsigned short port, const char *photoBaseFolderPath, const char *targetsFolderPath);
BOOL StartFrokAgent(FrokAgentContext *context);
BOOL StopFrokAgent(FrokAgentContext *context);
BOOL DeinitFrokAgent(FrokAgentContext context);

/*
#pragma pack(push, 1)

typedef struct
{
    SOCKET      replySocket;
    unsigned    dataLength;
    char       *data;
} FaceRequest;

#pragma pack(pop)

class FrokAgent
{
private:
    char                   *photoBasePath;
    char                   *targetsFolderPath;
    unsigned short          localPortNumber;
    SOCKET                  localSock;
    FaceRecognizerAbstract *recognizer;
    FaceDetectorAbstract   *detector;
    FrokAPI                *fapi;

public:
    FrokAgent(std::map<std::string, FrokAPIFunction*> enabledFucntions, unsigned short localPort = DEFAULT_PORT, const char *photoBasePath = DEFAULT_PHOTO_BASE_PATH, const char *targetsFolderPath = DEFAULT_TARGETS_FOLDER_PATH);
    ~FrokAgent();

    bool StartFrokAgent();
    bool StopFrokAgent();
protected:
    NetResult StartNetworkServer();
    NetResult StopNetworkServer();
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    // Only one connected server is allowed. Disconnect current connection to allow new server connect
    void ServerListener();
};*/

#endif // FROKAGENT_H
