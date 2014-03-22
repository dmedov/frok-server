#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "EigenDetector_v2.h"


int saveLearnModel(){
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();
	char path_model[1024] = "", path[1024] = "C:\\Face_detector_OK\\tmp\\";

	//обучение FaceRecognizer
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
	IplImage *img = 0, *imageResults = 0, *face = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();
	Ptr<FaceRecognizer> model = createEigenFaceRecognizer(5, 2500);

	char yml_dir[1024];
	sprintf(yml_dir, "%s//%s", dir, "eigenface.yml");
	model->load(yml_dir);

	//CvCapture *capture = 0;
	//capture = cvCreateCameraCapture(CV_CAP_ANY);
	//img = cvQueryFrame(capture);	//получение изображения камеры

	img = cvLoadImage(img_dir);

	if (!img) {
		cerr << "image load error" << endl;
		return -1;
	}

	else imageResults = cvCloneImage(img);

	//Создание хранилища памяти
	storage = cvCreateMemStorage(0);
	violaJonesDetection->cascadeDetect(img, imageResults, face, storage, model);
	cvShowImage("img2", imageResults);

	while (1){ if (cvWaitKey(33) == 27)	break; }

	cvReleaseImage(&img);
	cvReleaseImage(&imageResults);
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

	cout << key << endl;

	if (key[1] == 'l'){		
		return saveLearnModel();
	}	else if (key[1] == 'r'){
		return recognizeFromModel(argv[1],"C:\\Face_detector_OK\\tmp\\");
	}
	else if (key[1] == 'f'){



		return 0;
	}
}