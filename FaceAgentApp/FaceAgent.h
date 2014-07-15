#ifndef FACESERVER_H
#define FACESERVER_H

// FaceServer defaults
#define DEFAULT_PHOTO_BASE_PATH     "/home/zda/faces/"
#define DEFAULT_TARGETS_FOLDER_PATH "/home/zda/faces/"
#define DEFAULT_PORT                28015

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"

// Return codes for Network class
typedef enum NetResult
{
    NET_SUCCESS                 = 0x00,
    NET_SOCKET_ERROR            = 0x01,
    NET_MEM_ALLOCATION_FAIL     = 0x02,
    NET_NO_CALLBACK             = 0x03,
    NET_UNSPECIFIED_ERROR       = 0x04,
    NET_INVALID_PARAM           = 0x05,
    NET_COMMON_THREAD_ERROR     = 0x06,
    NET_ALREADY_STARTED         = 0x07
    //...
} NetResult;

#pragma pack(push, 1)

typedef struct
{
    SOCKET      replySocket;
    unsigned    dataLength;
    char       *data;
} FaceRequest;

class FaceAgent
{
private:
    char                   *photoBasePath;
    char                   *targetsFolderPath;
    unsigned short          localPortNumber;
    SOCKET                  localSock;
    CommonThread           *threadAcceptConnection;

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
protected:
    NetResult StartNetworkServer();
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    static void CallbackListener(void *pContext);
    static void DefaultCallback(unsigned evt, SOCKET sock, unsigned length, void *param) {}

    // Accepts any incoming connection
    static void ServerListener(void* param);
    // Recieves any incoming information, and gives it to the upper layer
    static void SocketListener(void* param);
};

#pragma pack(pop)

#endif // FACESERVER_H
