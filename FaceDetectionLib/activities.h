#pragma once

#define CUT_TIMEOUT			(600000)
#define MAX_THREADS_NUM		(10)

#define ID_PATH				"C:\\OK\\tmp\\"
#define TARGET_PATH			"C:\\OK\\target\\"

#pragma pack(push, 1)
struct ContextForRecognize{
	SOCKET sock;
	string targetImg;
	json::Array arrFrinedsList;
};

struct ContextForTrain{
	SOCKET sock;
	json::Array arrIds;
};

struct ContextForGetFaces{
	SOCKET sock;
	string userId;
	string photoName;
};

struct ContextForSaveFaces{
	SOCKET sock;
	string userId;
	string photoName;
	json::Object faceCoords;
};

#pragma pack(pop)

DWORD recognizeFromModel(void *pContext);
DWORD generateAndTrainBase(void *pContext);
DWORD getFacesFromPhoto(void *pContext);
DWORD saveFaceFromPhoto(void *pContext);
