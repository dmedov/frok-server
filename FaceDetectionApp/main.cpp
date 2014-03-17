#include "stdafx.h"
#include "ViolaJonesDetection.h";
#include "EigenDetector.h";

void usage()
{
	cout << "Usage:" << endl << "<image name + path>" << endl << "example";
	return;
}

int main(int argc, char *argv[]) {
	CvMemStorage* storage = 0;
	IplImage *img = 0, *imageResults = 0;
	ViolaJonesDetection *violaJonesDetection = new ViolaJonesDetection();

	if (argc != 2)
	{
		cerr << "invalid input arguments" << endl;
		return -1;
	}

	//CvCapture *capture = 0;
	//capture = cvCreateCameraCapture(CV_CAP_ANY);

	//img = cvQueryFrame(capture);	//��������� ����������� ������

	img = cvLoadImage(argv[1]);

	if (!img) {
		cerr << "image load error" << endl;
		return -1;
	}
	else imageResults = img;

	//�������� ��������� ������
	storage = cvCreateMemStorage(0);
	cvClearMemStorage(storage);
	violaJonesDetection->cascadeDetect(img, imageResults, storage);
	cvShowImage("img1", imageResults);

	while (1){
		char c = cvWaitKey(33);
		if (c == 27) { // ������ ESC
			//������� �� �����
			delete violaJonesDetection;
			cvDestroyAllWindows();
			return 0;
		}
	}
}
