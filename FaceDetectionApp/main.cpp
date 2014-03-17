#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "EigenDetector.h"

void usage()
{
	cout << "Usage:" << endl << "<image name + path>" << endl 
		<< "example: FaceDetectionApp.exe C:\\Face_detector_OK\\test_photo\\18.jpg";
	return;
}


int main(int argc, char *argv[]) {


	//unsigned int i = 0xABCD;
	//unsigned char c[4];
	//memcpy(c, &i, 4);

	CvMemStorage* storage = 0;
	IplImage *img = 0, *imageResults = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();

	if (argc != 2)
	{
		cerr << "invalid input arguments" << endl;
		usage();
		return -1;
	}

	//CvCapture *capture = 0;
	//capture = cvCreateCameraCapture(CV_CAP_ANY);

	//img = cvQueryFrame(capture);	//получение изображения камеры

	img = cvLoadImage(argv[1]);

	if (!img) {
		cerr << "image load error" << endl;
		return -1;
	}
	else imageResults = img;

	//Создание хранилища памяти
	storage = cvCreateMemStorage(0);
	cvClearMemStorage(storage);
	violaJonesDetection->cascadeDetect(img, imageResults, storage);
	cvShowImage("img1", imageResults);

	while (1){
		char c = cvWaitKey(33);
		if (c == 27) { // нажата ESC
			//удалить за собой
			delete violaJonesDetection;
			cvDestroyAllWindows();
			return 0;
		}
	}
}
