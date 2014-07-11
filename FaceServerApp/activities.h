/*#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include "string.h"
#include "json.h"

#define CUT_TIMEOUT            (600)
#define MAX_THREADS_AND_CASCADES_NUM        (1)


#pragma pack(push, 1)

struct ContextForRecognize
{
    SOCKET sock;
    std::string targetImg;
    json::Array arrFrinedsList;
};

struct ContextForTrain
{
    SOCKET sock;
    json::Array arrIds;
};

struct ContextForGetFaces
{
    SOCKET sock;
    std::string userId;
    std::string photoName;
};

struct ContextForSaveFaces
{
    SOCKET sock;
    std::string userId;
    std::string photoName;
    int faceNumber;
};

#pragma pack(pop)

void recognizeFromModel(void *pContext);
void generateAndTrainBase(void *pContext);
void getFacesFromPhoto(void *pContext);
void saveFaceFromPhoto(void *pContext);

#endif // ACTIVITIES_H
*/

