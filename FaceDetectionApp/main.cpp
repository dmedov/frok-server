#include "stdafx.h"
#include "LibInclude.h"

#define	NET_CMD_RECOGNIZE	"recognize"
#define NET_CMD_LEARN		"learn"
#define NET_CMD_CUT			"cut"

#define ID_PATH				"D:\\HerFace\\Faces\\"
#define TARGET_PATH			"C:\\OK\\tmp\\"

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
	string id;
};

#pragma pack(pop)

int saveLearnModel(void *pContext){
	ContextForLearn *psContect = (ContextForLearn*)pContext;
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

	//обучение FaceRecognizer
	eigenDetector_v2->learn((((string)ID_PATH).append(psContect->id)).c_str());
	delete psContect;
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
			model->load(((string)(ID_PATH)).append(psContext->arrFrinedsList[i].operator std::string()));
		}
		catch (...)
		{
			FilePrintMessage(NULL, _WARN("Learn was not called for user %d. Continue..."));
			continue;
		}
		models[psContext->arrFrinedsList[i].operator std::string()] = model;
	}

	if (models.empty())
	{
		FilePrintMessage(NULL, _FAIL("No models loaded."));
		return -1;
	}
	
	img = cvLoadImage(((string)(TARGET_PATH)).append(psContext->targetImg).c_str());

	if (!img) {
		FilePrintMessage(NULL, _FAIL("Failed to load image %s"), ((string)(TARGET_PATH)).append(psContext->targetImg).c_str());
		return -1;
	}
	else{
		storage = cvCreateMemStorage(0);					// Создание хранилища памяти
		violaJonesDetection->faceDetect(img, models);
	}

	while (1){
		if (cvWaitKey(0) == 27)
			break;
	}

	cvReleaseImage(&img);
	cvClearMemStorage(storage);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	delete psContext;
	return 0;
}

int cutFaces(void *pContext){
	ContextForCutFace *psContext = (ContextForCutFace*) pContext;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	_finddata_t result;

	for (unsigned i = 0; i < psContext->arrIds.size(); i++)
	{
		memset(&result, 0, sizeof(result));
		string photoName = ((string)ID_PATH).append(psContext->arrIds[i].operator std::string()).append("\\photos\\*.jpg");

		intptr_t firstHandle = _findfirst(photoName.c_str(), &result);
		if (firstHandle != -1)
		{
			do
			{
				photoName = ((string)ID_PATH).append(psContext->arrIds[i].operator std::string()).append("\\photos\\").append(result.name);
				IplImage *img = cvLoadImage(photoName.c_str());
				if (img == NULL)
				{
					FilePrintMessage(NULL, _WARN("Failed to load image %s. Continue..."), photoName.c_str());
					continue;
				}

				cout << "Cutting face from image " << result.name;

				violaJonesDetection->cutFace(img, ((string)ID_PATH).append(psContext->arrIds[i].operator std::string()).append("\\faces\\").append(result.name).c_str());

				cvReleaseImage(&img);
				cout << endl;

			} while (_findnext(firstHandle, &result) == 0);
		}
	}
	
	cvDestroyAllWindows();
	delete violaJonesDetection;
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
				psContext->arrFrinedsList = objInputJson["friends"].operator json::Array();
				psContext->targetImg = objInputJson["photo_id"];
				psContext->sock = sock;

				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recognizeFromModel, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else if (objInputJson["cmd"].operator string() == NET_CMD_LEARN)
			{
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
				psContext->arrIds = objInputJson["ids"].operator json::Array();
				psContext->sock = sock;

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

int main(int argc, char *argv[]) 
{
	net.StartNetworkServer(callback, PORT);
	
	//json::Value v = json::Deserialize("{\n  \"a\": 1,\n  \"b\": 2\n}");
	//char param[] = "{\"cmd\":\"cut\", \"ids\":[\"1\", \"2\"]}\0";		// cut faces
	char param[] = "{\"cmd\":\"recognize\", \"friends\":[\"1\"], \"photo_id\": \"1\"}\0";
	
	callback(1, NET_RECEIVED_REMOTE_DATA, strlen(param), param);
	getchar();

	/*if (argc != 3 && argc != 4){
		cerr << "invalid input arguments" << endl;
		return -1;
		}
		char *key = argv[2];

		//<Путь до папки с id> -l
		//C:\OK\tmp\ -l
		if (key[1] == 'l'){
		if (argc != 4) argv[3] = "-1";
		return saveLearnModel(argv[1], argv[3]);
		}
		//<Путь до изображения> -r <путь до *.yml>
		//C:\OK\test_photos\36.jpg -r C:\OK\tmp\

		else if (key[1] == 'r'){
		return recognizeFromModel(argv[1], argv[3]);
		}
		// <Путь до папки с fotos> -f
		// C:\OK\tmp\5\ -f
		else if (key[1] == 'f'){
		if (argc != 4) argv[3] = "-1";
		return rejectFaceForLearn(argv[1], argv[3]);
		}*/
	return 0;
}