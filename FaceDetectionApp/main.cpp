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
		model->load(((string)(ID_PATH)).append(psContext->arrFrinedsList[i].operator std::string()));
		models[psContext->arrFrinedsList[i].operator std::string()] = model;
	}
	
	img = cvLoadImage(((string)(TARGET_PATH)).append(psContext->targetImg).c_str());

	if (!img) {
		system("cls");
		cerr << "image load error" << endl;
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
					cerr << "image load error: " << photoName << endl;
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
				printf("Failed to parse incoming JSON: %s", (char*)param);
				return;
			}
			
			if (!objInputJson.HasKey("cmd"))
			{
				printf("Invalid input JSON: no cmd field\n");
				net.SendData(sock, "Error. Invalid JSON received: no cmd field\n\0", strlen("Error. Invalid JSON received: no cmd field\n\0"));
				return;
			}

			// Parse cmd
			if (objInputJson["cmd"].operator string() == NET_CMD_RECOGNIZE)
			{
				if (!objInputJson.HasKey("friends"))
				{
					printf("Invalid input JSON: no friends field\n");
					net.SendData(sock, "Error. Invalid JSON received: no friends field\n\0", strlen("Error. Invalid JSON received: no friends field\n\0"));
					return;
				}
				
				if (!objInputJson.HasKey("photo_id"))
				{
					printf("Invalid input JSON: no frineds field\n");
					net.SendData(sock, "Error. Invalid JSON received: no frineds field\n\0", strlen("Error. Invalid JSON received: no frineds field\n\0"));
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
					printf("Invalid input JSON: no ids field\n");
					net.SendData(sock, "Error. Invalid JSON received: no ids field\n\0", strlen("Error. Invalid JSON received: no ids field\n\0"));
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
				printf("Invalid input JSON: invalid cmd received\n");
				net.SendData(sock, "Error. Invalid JSON received: invalid cmd\n\0", strlen("Error. Invalid JSON received: invalid cmd\n\0"));
				return;
			}
			break;
		}
		default:
		{
			printf("Unknown event");
			break;
		}
	}
	return;
}

int main(int argc, char *argv[]) 
{
	net.StartNetworkServer(callback, PORT);
	
	//json::Value v = json::Deserialize("{\n  \"a\": 1,\n  \"b\": 2\n}");
	char param[] = "{\"cmd\":\"cut\", \"ids\":[\"1\", \"2\"]}\0";

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