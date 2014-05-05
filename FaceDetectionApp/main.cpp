#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "EigenDetector_v2.h"
#include "network.h"
#include <io.h>
#include "json.h"
Network net;

struct ContextForRecognize{
	SOCKET sock;
	char *dir_img;
	char *dir;
};

int saveLearnModel(char* dir_tmp, char* id){
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();
	char path_model[1024] = "";

	//обучение FaceRecognizer
	eigenDetector_v2->learn(dir_tmp, id);
	delete eigenDetector_v2;
	return 0;
}

int recognizeFromModel(void *pContext){    //SOCKET sock, char *img_dir, char* dir
	ContextForRecognize *psContext = (ContextForRecognize*)pContext;
	CvMemStorage* storage = 0;
	IplImage *img = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	vector <Ptr<FaceRecognizer>> models;

	char path_id[1024], yml_dir[1024];

	WIN32_FIND_DATA FindFileData;
	HANDLE hf;

	sprintf(path_id, "%s//*", psContext->dir);
	hf = FindFirstFile(path_id, &FindFileData);
	if (hf != INVALID_HANDLE_VALUE){
		while (FindNextFile(hf, &FindFileData) != 0){
			char* name = FindFileData.cFileName;
			if (strcmp(name, "..")){
				Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
				sprintf(yml_dir, "%s\\%s\\%s", psContext->dir, name, "eigenface.yml");
				model->load(yml_dir);
				models.push_back(model);
			}
		}
		FindClose(hf);
	}

	img = cvLoadImage(psContext->dir_img);

	if (!img) {
		system("cls");
		cerr << "image load error" << endl;
	}
	else{
		storage = cvCreateMemStorage(0);										//Создание хранилища памяти
		violaJonesDetection->dir = psContext->dir;
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

int rejectFaceForLearn(char* dir_tmp, char *id){
	IplImage *img = 0, *imageResults = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();


	_finddata_t result;
	char name[1024];
	long done;
	IplImage *base_face = 0;

	sprintf(name, "%s\\%s\\photos\\*.jpg", dir_tmp, id);
	memset(&result, 0, sizeof(result));
	done = _findfirst(name, &result);


	int nFaces = 0;
	if (done != -1)
	{
		int res = 0;
		while (res == 0){

			char image_dir[1024];
			if (strcmp(id, "-1")){
				sprintf(name, "%s\\%s\\photos\\%s", dir_tmp, id, result.name);
				sprintf(image_dir, "%s\\%s\\", dir_tmp, id);

				img = cvLoadImage(name);

				if (!img) {
					cerr << "image load error: " << name << endl;
					return -1;
				}

				cout << result.name;


				violaJonesDetection->dir = image_dir;
				violaJonesDetection->rejectFace(img, result.name);
				cout << endl;
			}
			res = _findnext(done, &result);
		}
	}

	cvReleaseImage(&img);
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
					ContextForRecognize *psContext = new ContextForRecognize;
					memset(psContext, 0, sizeof(ContextForRecognize));
					psContext->dir = new char[strlen("C:\\OK\\tmp\0")];
					memset(psContext->dir, 0, strlen("C:\\OK\\tmp\0"));
					strcpy(psContext->dir, "C:\\OK\\tmp\0");
					psContext->dir_img = new char[strlen("C:\\OK\\test_photos\\36.jpg\0")];
					memset(psContext->dir_img, 0, strlen("C:\\OK\\test_photos\\36.jpg\0"));
					strcpy(psContext->dir_img, "C:\\OK\\test_photos\\36.jpg\0");
					psContext->sock = sock;

					// recognizeFromModel(argv[1], argv[3])
					CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recognizeFromModel, psContext, 0, NULL);

					//printf("Received data: %s\n", (char*)param);
					//// command will be added with JSON
					//
					//NetworkCommand cmd = (NetworkCommand)0;
					//// cmd = parseFromJSON((char*)param);
					//switch(cmd)
					//{
					//	case NET_CMD_RECOGNIZE:
					//	{
					//		// call recognize
					//		SOCKET *socketForFunc = new SOCKET;
					//		memcpy_s(socketForFunc, sizeof(SOCKET), &sock, sizeof(SOCKET));
					//		printf("Starting thread...\n");
					//		
					//		break;
					//	}
					//	case NET_CMD_LEARN:
					//	{
					//		// call learn
					//		break;
					//	}
					//}
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

int main(int argc, char *argv[]) {

	String jsonString = "// Configuration options{// Default encoding for text\"encoding\" : \"UTF-8\",	// Plug-ins loaded at start-up\"plug-ins\" : [\"python\",\"c++\",\"ruby\"],// Tab indent size\"indent\" : { \"length\" : 3, \"use_space\" : true }}";
	Json::Value value;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(jsonString, value, false);
	if (!parsingSuccessful)
	{
		// report to the user the failure and their locations in the document.
		std::cout << "Failed to parse configuration\n";
		return -1;
	}

	// Get the value of the member of root named 'encoding', return 'UTF-8' if there is no
	// such member.
	std::string encoding = value.get("encoding", "UTF-8").asString();
	// Get the value of the member of root named 'encoding', return a 'null' value if
	// there is no such member.

	// And you can write to a stream, using the StyledWriter automatically.
	std::cout << value;

	/*
	net.StartNetworkServer(callback, PORT);

	getchar();*/

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
}