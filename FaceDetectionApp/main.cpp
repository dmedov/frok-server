#include "stdafx.h"
#include "ViolaJonesDetection.h";


int main() {
	CvMemStorage* storage = 0;
	IplImage *img = 0, *imageResults = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();

	img = cvLoadImage("C:\\Face_detector_OK\\test_photo\\15.jpg");

	if (!img) {
		cerr << "image load error" << endl;
		return -1;
	}
	else imageResults = img;

	//—оздание хранилища пам€ти
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
