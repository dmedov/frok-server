#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "EigenDetector_v2.h"

#include <io.h>


int saveLearnModel(){
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();
	char path_model[1024] = "", path[1024] = "C:\\Face_detector_OK\\tmp\\";

	//�������� FaceRecognizer
	Ptr<FaceRecognizer> model = createEigenFaceRecognizer(5, 2500);
	model = eigenDetector_v2->learn(path, model);

	sprintf(path_model, "%s//%s", path, "eigenface.yml");
	model->save(path_model);
	cout << path_model << " has been saved." << endl;

	delete eigenDetector_v2;
	return 0;
}

int recognizeFromModel(char *img_dir, char* dir){
	CvMemStorage* storage = 0;
	IplImage *img = 0, *imageResults = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	Ptr<FaceRecognizer> model = createEigenFaceRecognizer(5, 2500);

	char yml_dir[1024];
	sprintf(yml_dir, "%s//%s", dir, "eigenface.yml");
	model->load(yml_dir);
	
	img = cvLoadImage(img_dir);

	if (!img) {
		cerr << "image load error" << endl;
		return -1;
	}

	else imageResults = cvCloneImage(img);
	
	storage = cvCreateMemStorage(0);										//�������� ��������� ������

	violaJonesDetection->cascadeDetect(img, imageResults, storage, model);
	cvShowImage("img2", imageResults);

	while (1){ if (cvWaitKey(33) == 27)	break; }

	cvReleaseImage(&img);
	cvReleaseImage(&imageResults);
	cvClearMemStorage(storage);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	return 0;
}


int rejectFaceForLearn(char* dir){
	CvMemStorage* storage = 0;
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
			cout << r_name << endl;

			storage = cvCreateMemStorage(0);										//�������� ��������� ������
			violaJonesDetection->rejectFace(img, storage, dir, r_name);
			res = _findnext(done, &result);
		}
	}

	cvReleaseImage(&img);
	cvClearMemStorage(storage);
	cvDestroyAllWindows();
	delete violaJonesDetection;
	return 0;
}


int main(int argc, char *argv[]) {
	if (argc != 3){
		cerr << "invalid input arguments" << endl;
		return -1;
	}
	char *key = argv[2];

	if (key[1] == 'l'){		
		return saveLearnModel();
	}	else if (key[1] == 'r'){
		return recognizeFromModel(argv[1],"C:\\Face_detector_OK\\tmp\\");
	}
	else if (key[1] == 'f'){

		return rejectFaceForLearn(argv[1]);
	}
}