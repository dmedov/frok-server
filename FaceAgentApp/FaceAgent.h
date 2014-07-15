#ifndef FACESERVER_H
#define FACESERVER_H

// FaceAgent defaults
#define DEFAULT_PHOTO_BASE_PATH     "/home/zda/faces/"
#define DEFAULT_TARGETS_FOLDER_PATH "/home/zda/faces/"
#define DEFAULT_PORT                28015
#define MAX_SOCKET_BUFF_SIZE            (163840)

// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"

// FaceAgent logging system
#ifdef FACE_AGENT_TRACE_ENABLED
#define FACE_AGENT_TRACE(__function_name__, format, ...)    \
    pthread_mutex_lock(&faceAgent_trace_cs);                \
    printf("[FACE_AGENT->%s]: ", #__function_name__);       \
    printf(format, ##__VA_ARGS__);                          \
    printf("\n");                                           \
    pthread_mutex_unlock(&faceAgent_trace_cs)
#else
#define FACE_AGENT_TRACE(__function_name__, format, ...)
#endif

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
    CommonThread           *threadServerListener;

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
    NetResult StopNetworkServer();
    NetResult SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize);
private:
    // Only one connected server is allowed. Disconnect current connection to allow new server connect
    static void ServerListener(void* param);
};

#pragma pack(pop)

#endif // FACESERVER_H
