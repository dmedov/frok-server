#include "stdafx.h"
#include "cv.h"
#include "highgui.h"

#include <windows.h>
#include <iostream> 

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/nonfree.hpp"


using namespace std;
using namespace cv;

static CvHaarClassifierCascade* cascade = 0;
static CvHaarClassifierCascade* nested_cascade = 0;

static CvMemStorage* storage = 0;

const char* cascade_name =			"C:\\opencv\\data\\haarcascades\\haarcascade_frontalface_alt.xml";
const char* nested_cascade_name =	"C:\\opencv\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml";

void matchDescriptors(Mat ffd, Mat bfd){
	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	matcher.match(ffd, bfd, matches);

	double max_dist = 0; double min_dist = 2;

	//-- Quick calculation of max and min distances between keypoints
	for( int i = 0; i < ffd.rows; i++ )	{ 
		double dist = matches[i].distance;
		if( dist < min_dist ) min_dist = dist;
		if( dist > max_dist ) max_dist = dist;
	}

	std::vector< DMatch > good_matches;
	int c = 0;
	for( int i = 0; i < ffd.rows; i++ )	{ 
		if( matches[i].distance <= 2*min_dist )	{ 
			good_matches.push_back( matches[i]); 
			c++;
		}
	}
	cout << c << endl;

}

Mat findDescriptors(IplImage *face, char* name){
	//-- Этап 1. Нахождение ключевых точек.
	SurfFeatureDetector detector(400);	
	vector<KeyPoint> keypoints;
	detector.detect( face, keypoints );
	Mat img_keypoints, descriptors_object;
	drawKeypoints(face, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

	//-- Этап 2. Вычисление дескрипторов.
	SurfDescriptorExtractor extractor;
	extractor.compute(face, keypoints, descriptors_object);	

	imshow(name, img_keypoints );
	return descriptors_object;
}

void cascadeDetect(CvHaarClassifierCascade* cscd, IplImage* image, CvMemStorage* strg){
	IplImage 
		*gray_img = 0, 
		*small_img = 0,
		*base_face = 0, 
		*find_face = 0;

	gray_img = cvCreateImage( cvGetSize(image), 8, 1 );
	cvCvtColor(image, gray_img, CV_BGR2GRAY);

	if(cscd){			
		CvSeq *faces = cvHaarDetectObjects(gray_img, cscd, strg, 3, 2, 0 | CV_HAAR_DO_CANNY_PRUNING, cvSize(30, 30));
		for(int i = 0; i < (faces ? faces->total : 0); i++ )
		{
			CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
			int x = cvRound(r->x);
			int y = cvRound(r->y);
			int w = cvRound(r->width);
			int h = cvRound(r->height);
			CvPoint p1,p2;
			p1.x = x;
			p1.y  = y;
			p2.x = x + w;
			p2.y = y + h;				
			cvRectangle(image, p1, p2,CV_RGB(255,255,255) );
			small_img = cvCloneImage(gray_img);
			cvSetImageROI(small_img, cvRect(x, y, w, h));

			base_face = cvLoadImage("C:\\Users\\Dh\\Documents\\Visual Studio 2010\\Projects\\FaceDetect\\FaceDetect\\fc.jpg",1);
			find_face  =  cvCreateImage(cvSize(base_face ->width, base_face->height),8,1);
			cvResize(small_img,find_face);


			cvShowImage("face", find_face);
			//Mat ffDescriptors = findDescriptors(find_face, "find_face");
			//Mat bfDescriptors = findDescriptors(base_face, "base_face");

			//matchDescriptors(ffDescriptors, bfDescriptors);
		}
	}


	// освобождаем ресурсы
	cvReleaseImage(&gray_img);
	cvReleaseImage(&small_img);
	cvReleaseImage(&base_face);
	cvReleaseImage(&find_face);
}



int main(int argc, char* argv[]) {
	CvCapture *capture0 = 0;
	IplImage *img = 0;
	capture0 = cvCreateCameraCapture(1);


	cvSetCaptureProperty(capture0, CV_CAP_PROP_FRAME_WIDTH, 640);
	cvSetCaptureProperty(capture0, CV_CAP_PROP_FRAME_HEIGHT, 480);


	//Загрузка базы данных, обученной на детектирование лиц в Фас
	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
	//Загрузка быза данных, обученной для детектирования глаз
	nested_cascade = (CvHaarClassifierCascade*)cvLoad( nested_cascade_name, 0, 0, 0);

	if(!cascade ) {
		cout << "exit" << endl;
		return 1;
	}
	//Создание хранилища памяти
	storage = cvCreateMemStorage(0);


	while(true){
		img = cvQueryFrame(capture0);	//получение изображения камеры
		cvClearMemStorage(storage);
		cascadeDetect(cascade,img,storage);

		cvShowImage("img",img);
		char c = cvWaitKey(33);
		if (c == 27) { // нажата ESC
			break;
		}
	}
	//удалить за собой
	cvDestroyAllWindows(); 
	cvReleaseCapture(&capture0);
	return 0;
}
