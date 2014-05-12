#include "stdafx.h"
#include "LibInclude.h"
#include <ctime>

#define	NET_CMD_RECOGNIZE	"recognize"
#define NET_CMD_LEARN		"learn"
#define NET_CMD_CUT			"cut"

#define ID_PATH				"D:\\HerFace\\Faces\\"
#define TARGET_PATH			"D:\\HerFace\\Faces\\1\\"

Network net;

#pragma pack(push, 1)
struct ContextForRecognize{
	SOCKET sock;
	string targetImg;
	json::Array arrFrinedsList;
};

struct ContextForCutFace{
	SOCKET sock;
	json::Array arrIds;
};

struct ContextForLearn{
	SOCKET sock;
	json::Array arrIds;
};

#pragma pack(pop)

int saveLearnModel(void *pContext){
	ContextForLearn *psContext = (ContextForLearn*)pContext;
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

	//обучение FaceRecognizer
	for (unsigned i = 0; i < psContext->arrIds.size(); i++)
	{
		eigenDetector_v2->learn((((string)ID_PATH).append(psContext->arrIds[i].ToString())).c_str());
	}

	net.SendData(psContext->sock, "{ \"success\":\"learn succeed\" }", strlen("{ \"success\":\"learn succeed\" }"));

	delete psContext;
	delete eigenDetector_v2;
	return 0;
}

int recognizeFromModel(void *pContext)
{
	ContextForRecognize *psContext = (ContextForRecognize*)pContext;
	CvMemStorage* storage = NULL;
	IplImage *img = NULL;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	map <string, Ptr<FaceRecognizer>> models;
	
	for (unsigned i = 0; i < psContext->arrFrinedsList.size(); i++)
	{
		Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
		try
		{
			model->load(((string)(ID_PATH)).append(psContext->arrFrinedsList[i].operator std::string().append("//eigenface.yml")));
		}
		catch (...)
		{
			FilePrintMessage(NULL, _WARN("Failed to load model base for user %d. Continue..."), psContext->arrFrinedsList[i].operator std::string());
			continue;
		}
		models[psContext->arrFrinedsList[i].operator std::string()] = model;
	}

	if (models.empty())
	{
		FilePrintMessage(NULL, _FAIL("No models loaded."));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}
	
	img = cvLoadImage(((string)(TARGET_PATH)).append(psContext->targetImg.append(".jpg")).c_str());

	if (!img)
	{
		FilePrintMessage(NULL, _FAIL("Failed to load image %s"), ((string)(TARGET_PATH)).append(psContext->targetImg).c_str());
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}
	
	storage = cvCreateMemStorage(0);					// Создание хранилища памяти
	violaJonesDetection->faceDetect(img, models, psContext->sock);

	while (1){
		if (cvWaitKey(0) == 27)
			break;
	}

	net.SendData(psContext->sock, "{ \"success\":\"recognize faces succeed\" }", strlen("{ \"success\":\"recognize faces succeed\" }"));

	cvReleaseImage(&img);
	cvClearMemStorage(storage);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	delete psContext;
	return 0;
}

DWORD cutFaces(void *pContext)
{
	double startTime = clock();

	ContextForCutFace *psContext = (ContextForCutFace*)pContext;
	_finddata_t result;
	HANDLE *phEventTaskCompleted = new HANDLE[psContext->arrIds.size()];
	std::vector <HANDLE> threads;
	
	for (unsigned i = 0; i < psContext->arrIds.size(); i++)
	{
		memset(&result, 0, sizeof(result));
		string photoName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\photos\\*.jpg");

		intptr_t firstHandle = _findfirst(photoName.c_str(), &result);

		if (firstHandle != -1)
		{
			do
			{
				photoName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\photos\\").append(result.name);
				IplImage *img = cvLoadImage(photoName.c_str());
				if (img == NULL)
				{
					FilePrintMessage(NULL, _WARN("Failed to load image %s. Continue..."), photoName.c_str());
					continue;
				}

				//cout << "Cutting face from image " << result.name;

				cutFaceThreadParams * param = new cutFaceThreadParams(img, (((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\faces\\").append(result.name)).c_str());
				threads.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)param->pThis->cutFaceThread, param, 0, NULL));

			} while (_findnext(firstHandle, &result) == 0);
		}
		
		if (!threads.empty())
		{
			DWORD res;
			if (WAIT_OBJECT_0 != (res = WaitForMultipleObjects(threads.size(), &threads[0], TRUE, INFINITE)))
			{
				FilePrintMessage(NULL, _FAIL("Timeout has occured during waiting for cutting images finished"));
				for (unsigned j = 0; j < threads.size(); j++)
				{
					TerminateThread(threads[j], -1);
				}
			}
			for (unsigned j = 0; j < threads.size(); j++)
			{
				CloseHandle(threads[j]);
			}
			threads.clear();
		}
	}

	cvDestroyAllWindows();
	net.SendData(psContext->sock, "{ \"success\":\"cut faces succeed\" }", strlen("{ \"success\":\"cut faces succeed\" }"));
	FilePrintMessage(NULL, _SUCC("Cutting succeed. Time elapsed %.4lf\n"), (clock() - startTime));
	return 0;
}

void callback(SOCKET sock, unsigned evt, unsigned length, void *param)
{
	if (sock == INVALID_SOCKET)
	{
		FilePrintMessage(NULL, _FAIL("Something went wrong. Msg received from invalid socket."));
		return;
	}
	switch (evt)
	{
		case
		NET_SERVER_CONNECTED:
		{
			printf("Received connection. Socket = 0x%x\n", sock);
			break;
		}
		case
		NET_SERVER_DISCONNECTED:
		{
			printf("Received disconnection. Socket = 0x%x\n", sock);
			break;
		}
		case
		NET_RECEIVED_REMOTE_DATA:
		{
			json::Object objInputJson;
			try
			{
				objInputJson = ((json::Value)json::Deserialize((string)((char*)param))).ToObject();
			}
			catch (...)
			{
				FilePrintMessage(NULL, _FAIL("Failed to parse incoming JSON: %s"), (char*)param);
				net.SendData(sock, "{ \"error\":\"bad command\" }", strlen("{ \"error\":\"bad command\" }"));
				return;
			}
			
			if (!objInputJson.HasKey("cmd"))
			{
				FilePrintMessage(NULL, _FAIL("Invalid input JSON: no cmd field (%s)"), (char*)param);
				net.SendData(sock, "{ \"error\":\"no cmd field\" }", strlen("{ \"error\":\"no cmd field\" }"));
				return;
			}

			// Parse cmd
			if (objInputJson["cmd"].operator string() == NET_CMD_RECOGNIZE)
			{
				if (!objInputJson.HasKey("friends"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no friends field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no friends field\" }", strlen("{ \"error\":\"no friends field\" }"));
					return;
				}
				
				if (!objInputJson.HasKey("photo_id"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no photo_id field\" }", strlen("{ \"error\":\"no photo_id field\" }"));
					return;
				}

				ContextForRecognize *psContext = new ContextForRecognize;
				memset(psContext, 0, sizeof(ContextForRecognize));
				psContext->arrFrinedsList = objInputJson["friends"].ToArray();
				psContext->targetImg = objInputJson["photo_id"];
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Recognizing started."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recognizeFromModel, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else if (objInputJson["cmd"].operator string() == NET_CMD_LEARN)
			{
				if (!objInputJson.HasKey("ids"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no ids field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no ids field\" }", strlen("{ \"error\":\"no ids field\" }"));
					return;
				}

				ContextForLearn *psContext = new ContextForLearn;
				memset(psContext, 0, sizeof(ContextForLearn));
				psContext->arrIds = objInputJson["ids"].ToArray();
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Learning started."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)saveLearnModel, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else if (objInputJson["cmd"].operator string() == NET_CMD_CUT)
			{
				if (!objInputJson.HasKey("ids"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no ids field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no ids field\" }", strlen("{ \"error\":\"no ids field\" }"));
					return;
				}

				ContextForCutFace *psContext = new ContextForCutFace;
				memset(psContext, 0, sizeof(ContextForCutFace));
				psContext->arrIds = objInputJson["ids"].ToArray();
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Cutting faces started."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cutFaces, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else
			{
				FilePrintMessage(NULL, _FAIL("Invalid input JSON: invalid cmd received (%s)"), (char*)param);
				net.SendData(sock, "{ \"error\":\"invalid cmd\" }", strlen("{ \"error\":\"invalid cmd\" }"));
				return;
			}
			break;
		}
		default
			:
		{
			FilePrintMessage(NULL, _FAIL("Unknown event 0x%x"), evt);
			break;
		}
	}
	return;
}

void usage()
{
	FilePrintMessage(NULL, _SUCC("FaceDetectionApp <Local port number>"));
	return;
}

int main(int argc, char *argv[]) 
{
	if (argc != 2)
	{
		FilePrintMessage(NULL, _FAIL("Invalid input patemeters."));
		usage();
		return -1;
	}
	InitializeCriticalSection(&faceDetectionCS);
	InitializeCriticalSection(&fileCS);
	
	unsigned uPort = atoi(argv[1]);

	FilePrintMessage(NULL, _SUCC("Starting network server with port = %d"), uPort);
	if (NET_SUCCESS != net.StartNetworkServer(callback, uPort))
	{
		FilePrintMessage(NULL, _FAIL("Failed to start network. For additional info build project with NET_DEBUG_PRINT flag enabled"));
		DeleteCriticalSection(&faceDetectionCS);
		DeleteCriticalSection(&fileCS);
		return -1;
	}
	FilePrintMessage(NULL, _SUCC("Network server started!"));
	
	char cut[] = "{\"cmd\":\"cut\", \"ids\":[\"1\"]}\0";		// cut faces
	char learn[] = "{\"cmd\":\"learn\", \"ids\":[\"1\"]}\0";	// Learn base
	char recognize[] = "{\"cmd\":\"recognize\", \"friends\":[\"1\"], \"photo_id\": \"1\"}\0";	// recognize name = 1.jpg

	//callback(1, NET_RECEIVED_REMOTE_DATA, strlen(cut), cut);
	//callback(1, NET_RECEIVED_REMOTE_DATA, strlen(learn), learn);
	//callback(1, NET_RECEIVED_REMOTE_DATA, strlen(recognize), recognize);
	getchar();

	cvReleaseHaarClassifierCascade(&faceCascades.face);
	cvReleaseHaarClassifierCascade(&faceCascades.eyes);
	cvReleaseHaarClassifierCascade(&faceCascades.nose);
	cvReleaseHaarClassifierCascade(&faceCascades.mouth);
	cvReleaseHaarClassifierCascade(&faceCascades.eye);
	cvReleaseHaarClassifierCascade(&faceCascades.righteye2);
	cvReleaseHaarClassifierCascade(&faceCascades.lefteye2);
	DeleteCriticalSection(&faceDetectionCS);
	DeleteCriticalSection(&fileCS);

	return 0;
}