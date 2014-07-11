#ifndef FACESERVER_H
#define FACESERVER_H

// FaceServer defaults
#define DEFAULT_PHOTO_BASE_PATH     "/home/zda/faces/"
#define DEFAULT_TARGETS_FOLDER_PATH "/home/zda/faces/"
#define DEFAULT_PORT                27015
// include dependencies
#include "../FaceCommonLib/faceCommonLib.h"

class FaceServer
{
public:
private:
    char *photoBasePath;
    char *targetsFolderPath;
public:
    FaceServer(char *photoBasePath = DEFAULT_PHOTO_BASE_PATH, char*targetsFolderPath = DEFAULT_TARGETS_FOLDER_PATH);
    ~FaceServer();

    bool StartFaceServer();
    bool StopFaceServer();

private:
    void callbackListener(void *pContext);
    void networkCallback(unsigned evt, SOCKET sock, unsigned length, void *param);

    void recognizeFromModel(void *pContext);
    void generateAndTrainBase(void *pContext);
    void getFacesFromPhoto(void *pContext);
    void saveFaceFromPhoto(void *pContext);

};

#endif // FACESERVER_H
