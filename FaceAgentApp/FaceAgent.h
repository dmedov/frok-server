#ifndef FACESERVER_H
#define FACESERVER_H

// FaceServer defaults
#define DEFAULT_PHOTO_BASE_PATH     "/home/zda/faces/"
#define DEFAULT_TARGETS_FOLDER_PATH "/home/zda/faces/"
#define DEFAULT_PORT                28015

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"

#pragma pack(push, 1)

typedef struct
{
    SOCKET      replySocket;
    unsigned    dataLength;
    char       *data;
} FaceRequest;

class FaceAgent : public Network
{
private:
    Network            *network;
    char               *photoBasePath;
    char               *targetsFolderPath;
    unsigned short      localPort;
    CommonThread       *threadCallbackListener;

public:
    FaceAgent(unsigned short localPort = DEFAULT_PORT, char *photoBasePath = DEFAULT_PHOTO_BASE_PATH, char*targetsFolderPath = DEFAULT_TARGETS_FOLDER_PATH);
    ~FaceAgent();

    bool StartFaceAgent();
    bool StopFaceAgent();

// Activities
    void Recognize(void *pContext);
    void TrainUserModel(void *pContext);
    void GetFacesFromPhoto(void *pContext);
    void AddFaceFromPhoto(void *pContext);
private:
    static void CallbackListener(void *pContext);
    static void DefaultCallback(unsigned evt, SOCKET sock, unsigned length, void *param) {}
};

#pragma pack(pop)

#endif // FACESERVER_H
