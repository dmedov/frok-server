#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "EigenDetector_v2.h"

#include <io.h>


int saveLearnModel(char* dir_tmp, char* id){
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();
	char path_model[1024] = "";

	//�������� FaceRecognizer
	eigenDetector_v2->learn(dir_tmp, id);
	delete eigenDetector_v2;
	return 0;
}

int recognizeFromModel(char *dir_img, char* dir_tmp){
	CvMemStorage* storage = 0;
	IplImage *img = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	vector <Ptr<FaceRecognizer>> models;

	char path_id[1024], yml_dir[1024];

	WIN32_FIND_DATA FindFileData;
	HANDLE hf;

	sprintf(path_id, "%s//*", dir_tmp);
	hf = FindFirstFile(path_id, &FindFileData);
	if (hf != INVALID_HANDLE_VALUE){
		while (FindNextFile(hf, &FindFileData) != 0){
			char* name = FindFileData.cFileName;
			if (strcmp(name, "..")){
				Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
				sprintf(yml_dir, "%s\\%s\\%s", dir_tmp, name, "eigenface.yml");
				model->load(yml_dir);
				models.push_back(model);
			}
		}
		FindClose(hf);
	}


	img = cvLoadImage(dir_img);

	if (!img) {
		system("cls");
		cerr << "image load error" << endl;
	}
	else{
		storage = cvCreateMemStorage(0);										//�������� ��������� ������
		violaJonesDetection->dir = dir_tmp;
		violaJonesDetection->faceDetect(img, models);
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
				sprintf(name, "%s\\%s\\photos\\%s", dir_tmp,id,result.name);
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

int main(int argc, char *argv[]) {
	if (argc != 3 && argc != 4){
		cerr << "invalid input arguments" << endl;
		return -1;
	}

	char *key = argv[2];

	//<���� �� ����� � id> -l
	//C:\OK\tmp\ -l
	if (key[1] == 'l'){
		if (argc != 4) argv[3] = "-1";
		return saveLearnModel(argv[1], argv[3]);
	}
	//<���� �� �����������> -r <���� �� *.yml>
	//C:\OK\test_photos\36.jpg -r C:\OK\tmp\

	else if (key[1] == 'r'){
		return recognizeFromModel(argv[1], argv[3]);
	}
	// <���� �� ����� � fotos> -f		
	// C:\OK\tmp\5\ -f
	else if (key[1] == 'f'){
		if (argc != 4) argv[3] = "-1";
		return rejectFaceForLearn(argv[1], argv[3]);
	}
}