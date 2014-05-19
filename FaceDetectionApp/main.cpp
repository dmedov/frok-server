#include "stdafx.h"
#include "LibInclude.h"
#include <ctime>
// Add SHOW_IMAGE define to preprocessor defines in FaceDetectionApp and FaceDetectionLib projects to see resulting image
#define CUT_TIMEOUT			(600000)
#define MAX_THREADS_NUM		(10)
#define	NET_CMD_RECOGNIZE	"recognize"
#define NET_CMD_TRAIN		"train"

#define ID_PATH				"D:\\HerFace\\Faces\\"
#define TARGET_PATH			"D:\\HerFace\\Faces\\"

Network net;

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

#pragma pack(pop)

int recognizeFromModel(void *pContext)
{
	double startTime = clock();
	ContextForRecognize *psContext = (ContextForRecognize*)pContext;
	CvMemStorage* storage = NULL;
	IplImage *img = NULL;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	map <string, Ptr<FaceRecognizer>> models;
	
	for (UINT_PTR i = 0; i < psContext->arrFrinedsList.size(); i++)
	{
		Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
		try
		{
			model->load(((string)(ID_PATH)).append(psContext->arrFrinedsList[i].operator std::string()).append("//eigenface.yml"));
		}
		catch (...)
		{
			FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), psContext->arrFrinedsList[i].ToString().c_str());
			continue;
		}
		models[psContext->arrFrinedsList[i].operator std::string()] = model;
	}

	if (models.empty())
	{
		FilePrintMessage(NULL, _FAIL("No models loaded."));
		net.SendData(psContext->sock, "{ \"error\":\"training was not called\" }\n\0", strlen("{ \"error\":\"training was not called\" }\n\0"));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}
	
	img = cvLoadImage(((string)(TARGET_PATH)).append(psContext->targetImg.append(".jpg")).c_str());

	if (!img)
	{
		FilePrintMessage(NULL, _FAIL("Failed to load image %s"), (((string)(TARGET_PATH)).append(psContext->targetImg)).c_str());
		net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}
	
	storage = cvCreateMemStorage();					// Создание хранилища памяти
	try
	{
		violaJonesDetection->faceDetect(img, models, psContext->sock);
	}
	catch (...)
	{
		FilePrintMessage(NULL, _FAIL("Some error occured during recognze call"));
		net.SendData(psContext->sock, "{ \"error\":\"Recognize failed\" }\n\0", strlen("{ \"error\":\"Recognize failed\" }\n\0"));
		delete violaJonesDetection;
		delete psContext;
		return -1;
	}
	
#ifdef SHOW_IMAGE
	while (1){
		if (cvWaitKey(0) == 27)
			break;
	}
#endif

	net.SendData(psContext->sock, "{ \"success\":\"recognize faces succeed\" }\n\0", strlen("{ \"success\":\"recognize faces succeed\" }\n\0"));

	FilePrintMessage(NULL, _SUCC("Recognize finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
	cvReleaseImage(&img);
	cvClearMemStorage(storage);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	delete psContext;
	return 0;
}

DWORD generateAndTrainBase(void *pContext)
{
	double startTime = clock();

	ContextForTrain *psContext = (ContextForTrain*)pContext;
	_finddata_t result;
	HANDLE *phEventTaskCompleted = new HANDLE[psContext->arrIds.size()];
	std::vector <HANDLE> threads;
	UINT_PTR uSuccCounter = 0;

	for (UINT_PTR i = 0; i < psContext->arrIds.size(); i++)
	{
		memset(&result, 0, sizeof(result));
		string photoName = ((string)ID_PATH).append(psContext->arrIds[i].ToString()).append("\\photos\\*.jpg");

		intptr_t firstHandle = _findfirst(photoName.c_str(), &result);

		UINT_PTR uNumOfThreads = 0;

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
				threads.push_back(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)param->pThis->cutFaceThread, (LPVOID)param, 0, NULL));

				if (++uNumOfThreads > MAX_THREADS_NUM)
				{
					DWORD res;
					if (WAIT_OBJECT_0 != (res = WaitForMultipleObjects((unsigned)threads.size(), &threads[0], TRUE, CUT_TIMEOUT)))
					{
						FilePrintMessage(NULL, _FAIL("Timeout has occured during waiting for cutting images finished"));
						for (UINT_PTR j = 0; j < threads.size(); j++)
						{
							TerminateThread(threads.at(j), -1);
						}
					}

					for (UINT_PTR j = 0; j < threads.size(); j++)
					{
						CloseHandle(threads.at(j));
					}
					threads.clear();

					uNumOfThreads = 0;
				}

			} while (_findnext(firstHandle, &result) == 0);

			if (uNumOfThreads != 0)
			{
				DWORD res;
				if (WAIT_OBJECT_0 != (res = WaitForMultipleObjects((unsigned)threads.size(), &threads[0], TRUE, CUT_TIMEOUT)))
				{
					FilePrintMessage(NULL, _FAIL("Timeout has occured during waiting for cutting images finished"));
					for (UINT_PTR j = 0; j < threads.size(); j++)
					{
						TerminateThread(threads.at(j), -1);
					}
				}

				for (UINT_PTR j = 0; j < threads.size(); j++)
				{
					CloseHandle(threads.at(j));
				}
				threads.clear();

				uNumOfThreads = 0;
			}
		}

		EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

		//train FaceRecognizer
		try
		{
			if (!eigenDetector_v2->train((((string)ID_PATH).append(psContext->arrIds[i].ToString())).c_str()))
			{
				delete eigenDetector_v2;
				continue;
			}
		}
		catch (...)
		{
			FilePrintMessage(NULL, _FAIL("Some error has occured during Learn call."));
			delete eigenDetector_v2;
			continue;
		}
		delete eigenDetector_v2;
		uSuccCounter++;
	}

	cvDestroyAllWindows();
	FilePrintMessage(NULL, _SUCC("Train finished. Time elapsed %.4lf s\n"), (clock() - startTime) / CLOCKS_PER_SEC);
	if (uSuccCounter == 0)
	{
		net.SendData(psContext->sock, "{ \"fail\":\"learning failed\" }\n\0", strlen("{ \"fail\":\"learning failed\" }\n\0"));
		return -1;
	}

	net.SendData(psContext->sock, "{ \"success\":\"train succeed\" }\n\0", strlen("{ \"success\":\"train succeed\" }\n\0"));
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
				net.SendData(sock, "{ \"error\":\"bad command\" }\n\0", strlen("{ \"error\":\"bad command\" }\n\0"));
				return;
			}
			
			if (!objInputJson.HasKey("cmd"))
			{
				FilePrintMessage(NULL, _FAIL("Invalid input JSON: no cmd field (%s)"), (char*)param);
				net.SendData(sock, "{ \"error\":\"no cmd field\" }\n\0", strlen("{ \"error\":\"no cmd field\" }\n\0"));
				return;
			}

			// Parse cmd
			if (objInputJson["cmd"].ToString() == NET_CMD_RECOGNIZE)
			{
				if (!objInputJson.HasKey("friends"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no friends field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no friends field\" }\n\0", strlen("{ \"error\":\"no friends field\" }\n\0"));
					return;
				}
				
				if (!objInputJson.HasKey("photo_id"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
					return;
				}

				ContextForRecognize *psContext = new ContextForRecognize;
				memset(psContext, 0, sizeof(ContextForRecognize));
				psContext->arrFrinedsList = objInputJson["friends"].ToArray();
				psContext->targetImg = objInputJson["photo_id"];
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Recognizing started..."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recognizeFromModel, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else if (objInputJson["cmd"].ToString() == NET_CMD_TRAIN)
			{
				if (!objInputJson.HasKey("ids"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no ids field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no ids field\" }\n\0", strlen("{ \"error\":\"no ids field\" }\n\0"));
					return;
				}

				ContextForTrain *psContext = new ContextForTrain;
				memset(psContext, 0, sizeof(ContextForTrain));
				psContext->arrIds = objInputJson["ids"].ToArray();
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Training started..."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)generateAndTrainBase, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else
			{
				FilePrintMessage(NULL, _FAIL("Invalid input JSON: invalid cmd received (%s)"), (char*)param);
				net.SendData(sock, "{ \"error\":\"invalid cmd\" }\n\0", strlen("{ \"error\":\"invalid cmd\" }\n\0"));
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
	InitializeCriticalSection(&faceDetectionCS);
	InitializeCriticalSection(&fileCS);

	if (argc != 2)
	{
		FilePrintMessage(NULL, _FAIL("Invalid input patemeters."));
		usage();
		DeleteCriticalSection(&faceDetectionCS);
		DeleteCriticalSection(&fileCS);
		return -1;
	}
	
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
	
	/*char train[] = "{\"cmd\":\"train\", \"ids\":[\"6\"]}\0";	// cut faces and train base
	callback(1, NET_RECEIVED_REMOTE_DATA, strlen(train), train);*/
	char recognize[] = "{\"cmd\":\"recognize\", \"friends\":[\"6\"], \"photo_id\": \"11\"}\0";	// recognize name = 1.jpg
	callback(1, NET_RECEIVED_REMOTE_DATA, strlen(recognize), recognize);
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