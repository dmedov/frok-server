#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "EigenDetector_v2.h"

#include <io.h>


int saveLearnModel(char* dir, char* id){
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();
	char path_model[1024] = "";

	//обучение FaceRecognizer
	eigenDetector_v2->learn(dir, id);
	delete eigenDetector_v2;
	return 0;
}



int recognizeFromModel(char *img_dir, char* dir){
	CvMemStorage* storage = 0;
	IplImage *img = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	vector <Ptr<FaceRecognizer>> models;

	char path_id[1024], yml_dir[1024];

	WIN32_FIND_DATA FindFileData;
	HANDLE hf;

	sprintf(path_id, "%s//*", dir);
	hf = FindFirstFile(path_id, &FindFileData);
	if (hf != INVALID_HANDLE_VALUE){
		while (FindNextFile(hf, &FindFileData) != 0){
			char* name = FindFileData.cFileName;
			if (strcmp(name, "..")){
				Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
				sprintf(yml_dir, "%s\\%s\\%s", dir, name, "eigenface.yml");
				model->load(yml_dir);
				models.push_back(model);
			}
		}
		FindClose(hf);
	}


	img = cvLoadImage(img_dir);

	if (!img) {
		system("cls");
		cerr << "image load error" << endl;
	}
	else{
		storage = cvCreateMemStorage(0);										//Создание хранилища памяти
		violaJonesDetection->dir = dir;
		violaJonesDetection->cascadeDetect(img, models);
	}
	while (1){

		if (cvWaitKey(33) == 27)	break;
	}

	cvReleaseImage(&img);
	cvClearMemStorage(storage);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	return 0;
}

int rejectFaceForLearn(char* dir){
	IplImage *img = 0, *imageResults = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();


	_finddata_t result;
	char name[1024];
	long done;
	IplImage *base_face = 0;

	sprintf(name, "%s\\photos\\*.jpg", dir);
	memset(&result, 0, sizeof(result));
	done = _findfirst(name, &result);

	int nFaces = 0;
	if (done != -1)
	{
		int res = 0;
		while (res == 0){

			char* r_name = result.name;

			sprintf(name, "%s\\photos\\%s", dir, r_name);
			img = cvLoadImage(name);

			if (!img) {
				cerr << "image load error: " << name << endl;
				return -1;
			}

			cout << r_name;
			violaJonesDetection->dir = dir;
			violaJonesDetection->rejectFace(img,r_name);
			cout << endl;
			res = _findnext(done, &result);
		}
	}

	cvReleaseImage(&img);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3 && argc != 4){
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
		return rejectFaceForLearn(argv[1]);
	}
}