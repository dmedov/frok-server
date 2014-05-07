#include "stdafx.h"
#include "LibInclude.h"

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
			//json::Value v = json::Deserialize("{\n  \"a\": 1,\n  \"b\": 2\n}");
			json::Object objInputJson = ((json::Value)json::Deserialize(((string)((char*)param)))).operator json::Object;
			if (!objInputJson.HasKey("cmd"))
			{
				printf("Invalid input JSON: no cmd field\n");
				net.SendData(sock, "Error. Invalid JSON received: no cmd field\n\0", strlen("Error. Invalid JSON received: no cmd field\n\0"));
				return;
			}

			// Parse cmd
			switch (objInputJson["cmd"].operator int)
			{
				case
				NET_CMD_RECOGNIZE:
				{
				// Recognize smth
					ContextForRecognize *psContext = new ContextForRecognize;
					memset(psContext, 0, sizeof(ContextForRecognize));
					psContext->dir = new char[strlen("C:\\OK\\tmp\0")];
					memset(psContext->dir, 0, strlen("C:\\OK\\tmp\0"));
					strcpy(psContext->dir, "C:\\OK\\tmp\0");
					psContext->dir_img = new char[strlen("C:\\OK\\test_photos\\36.jpg\0")];
					memset(psContext->dir_img, 0, strlen("C:\\OK\\test_photos\\36.jpg\0"));
					strcpy(psContext->dir_img, "C:\\OK\\test_photos\\36.jpg\0");
					psContext->sock = sock;

					CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recognizeFromModel, psContext, 0, NULL);
					break;
				}
				case
				NET_CMD_LEARN:
				{
					// LYORN BLYA
					break;
				}
				default:
				{
						printf("Invalid input JSON: invalid cmd received\n");
						net.SendData(sock, "Error. Invalid JSON received: invalid cmd\n\0", strlen("Error. Invalid JSON received: invalid cmd\n\0"));
						return;
				}
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

int main(int argc, char *argv[]) {

	net.StartNetworkServer(callback, PORT);

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