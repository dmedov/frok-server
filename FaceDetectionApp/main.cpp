#include "cv.h"
#include "highgui.h"

#include <windows.h>
#include <iostream> 

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"


using namespace std;
using namespace cv;

static CvMemStorage* storage = 0;

//«агрузка базы данных, обученной на детектирование лиц в ‘ас
static CvHaarClassifierCascade* cascade =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\data\\haarcascades\\haarcascade_frontalface_alt.xml", 0, 0, 0);

//«агрузка быза данных, обученной дл€ детектировани€ глаз
static CvHaarClassifierCascade* cascade_eyes =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml", 0, 0, 0);

//«агрузка быза данных, обученной дл€ детектировани€ носа
static CvHaarClassifierCascade* cascade_nose =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\data\\haarcascades\\haarcascade_mcs_nose.xml", 0, 0, 0);

//«агрузка быза данных, обученной дл€ детектировани€ рота
static CvHaarClassifierCascade* cascade_mouth =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\data\\haarcascades\\haarcascade_mcs_mouth.xml", 0, 0, 0);


void matchDescriptors(Mat ffd, Mat bfd){
	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	matcher.match(ffd, bfd, matches);

	double max_dist = 0; double min_dist = 2;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < ffd.rows; i++)	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	std::vector< DMatch > good_matches;
	int c = 0;
	for (int i = 0; i < ffd.rows; i++)	{
		if (matches[i].distance <= 2 * min_dist)	{
			good_matches.push_back(matches[i]);
			c++;
		}
	}
	cout << c << endl;

}

Mat findDescriptors(IplImage *face, char* name){
	//-- Ётап 1. Ќахождение ключевых точек.
	SurfFeatureDetector detector(400);
	vector<KeyPoint> keypoints;
	detector.detect(face, keypoints);
	Mat img_keypoints, descriptors_object;
	drawKeypoints(face, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

	//-- Ётап 2. ¬ычисление дескрипторов.
	SurfDescriptorExtractor extractor;
	extractor.compute(face, keypoints, descriptors_object);

	imshow(name, img_keypoints);
	return descriptors_object;
}

void keysFaceDetect(CvHaarClassifierCascade* cscd, IplImage* imageResults, IplImage* imageFace, CvMemStorage* strg, CvPoint p, int type, CvPoint* facePoints){
	IplImage* dst = 0;
	int k;

	int w = imageFace->width;
	int h = imageFace->height;
	//ресайз картинки
	if (w < 100 || h < 100)	k = 5;
	else k = 1;

	w = w * k;
	h = h * k;
	dst = cvCreateImage(cvSize(w, h), imageFace->depth, imageFace->nChannels);
	cvResize(imageFace, dst, 1);
	

	CvSize minSize = cvSize(w / 4, h / 4);

	if (cscd){
		switch (type){
		case 0:
			minSize = cvSize(w / 6, h / 7);
			break;
		case 1:
			minSize = cvSize(w / 6, h / 6);
			break;
		case 2:
			minSize = cvSize(w / 5, h / 6);
			break;
		}


		CvSeq *objects = cvHaarDetectObjects(dst, cscd, strg, 1.1, 2, 0 | CV_HAAR_DO_CANNY_PRUNING, minSize);

		int count = 0;
		for (int i = 0; i < (objects ? objects->total : 0); i++)
		{
			CvRect* r = (CvRect*)cvGetSeqElem(objects, i);
			int x = cvRound(r->x) / k;
			int y = cvRound(r->y) / k;
			int w = cvRound(r->width) / k;
			int h = cvRound(r->height) / k;


			CvPoint p1 = cvPoint(x + p.x, y + p.y), p2 = cvPoint(x + w + p.x, y + h + p.y);
			if (type == 0){

				if (count == 0){
					//первый глаз
					facePoints[0] = p1;
					facePoints[4] = p2;
					cvRectangle(imageResults, p1, p2, CV_RGB(255, 128, 0));
				}
				else if (count == 1){
					//второй глаз
					facePoints[1] = p1;
					facePoints[5] = p2;
					cvRectangle(imageResults, p1, p2, CV_RGB(0, 128, 255));
				}
				count++;

			}
			else if (type == 1 
				&& p1.y >= (facePoints[0].y/* + (facePoints[4].y - facePoints[0].y) / 1.5*/)){
				// нос
				facePoints[2] = p1;
				facePoints[6] = p2;
				cvRectangle(imageResults, p1, p2, CV_RGB(255, 100, 255));
				break;
			}
			else if (type == 2 
				&& p1.y >= (facePoints[2].y/* + (facePoints[6].y - facePoints[2].y) / 1.5*/) 
				&& p1.y >= (facePoints[0].y + (facePoints[4].y - facePoints[0].y) / 2)){
				// рот
				facePoints[3] = p1;
				facePoints[7] = p2;
				cvRectangle(imageResults, p1, p2, CV_RGB(128, 0, 128));
				break;
			}
		}
	}

}

void cascadeDetect(CvHaarClassifierCascade* cscd, IplImage* image, IplImage *imageResults, CvMemStorage* strg){
	IplImage
		*gray_img = 0,
		*small_img = 0,
		*base_face = 0,
		*find_face = 0;

	gray_img = cvCreateImage(cvGetSize(image), 8, 1);
	cvCvtColor(image, gray_img, CV_BGR2GRAY);

	if (cscd){
		CvSeq *faces = cvHaarDetectObjects(gray_img, cscd, strg, 1.2, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, cvSize(40, 50));
		for (int i = 0; i < (faces ? faces->total : 0); i++)
		{
			CvRect* r = (CvRect*)cvGetSeqElem(faces, i);

			CvPoint p1, p2;
			CvPoint* facePoints = new CvPoint[8];

			int x = cvRound(r->x);			int y = cvRound(r->y);
			int w = cvRound(r->width);		int h = cvRound(r->height);

			p1 = cvPoint(x, y);
			p2 = cvPoint(x + w, y + h);

			small_img = cvCreateImage(cvSize(w, h), gray_img->depth, gray_img->nChannels);
			cvSetImageROI(gray_img, cvRect(x, y, w, h));
			cvCopy(gray_img, small_img, NULL);
			cvResetImageROI(gray_img);														//копируем лицо в отдельную картинку


			for (int i = 0; i < 8; i++)	facePoints[i] = cvPoint(-1, -1);						//по умолчанию координаты всех точек равны -1; -1

			keysFaceDetect(cascade_eyes, imageResults, small_img, storage, p1, 0, facePoints);
			keysFaceDetect(cascade_nose, imageResults, small_img, storage, p1, 1, facePoints);
			keysFaceDetect(cascade_mouth, imageResults, small_img, storage, p1, 2, facePoints);

			bool b = 1;

			for (int i = 0; i < 8; i++)												//провер€ем координаты всех точек на -1;-1
			if (facePoints[i].x < 0 || facePoints[i].y < 0)	b = 0;

			if (b){
				CvPoint centralCoords[4];

				for (int i = 0; i < 4; i++)
					centralCoords[i] = cvPoint((facePoints[i].x + facePoints[i + 4].x) / 2, (facePoints[i].y + facePoints[i + 4].y) / 2);

				for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					cvLine(imageResults, centralCoords[i], centralCoords[j], CV_RGB(0, 0, 255));
				cvRectangle(imageResults, p1, p2, CV_RGB(255, 255, 0));
			}
			else					cvRectangle(imageResults, p1, p2, CV_RGB(255, 255, 255));
		}
	}


	// освобождаем ресурсы
	cvReleaseImage(&gray_img);
	cvReleaseImage(&small_img);
	cvReleaseImage(&base_face);
	cvReleaseImage(&find_face);
}

int main() {
	IplImage* img = cvLoadImage("test_photo\\12.jpg");
	if (!cascade) {
		cout << "exit" << endl;
		return 1;
	}
	//—оздание хранилища пам€ти
	storage = cvCreateMemStorage(0);
	IplImage *imageResults = img;
	cvClearMemStorage(storage);
	cascadeDetect(cascade, img, imageResults, storage);
	cvShowImage("img2", imageResults);

	while (1){
		char c = cvWaitKey(33);
		if (c == 27) { // нажата ESC
			//удалить за собой
			cvDestroyAllWindows();
			return 0;
		}
	}

}
