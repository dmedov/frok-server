#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "SiftDetection.h"

#include <Math.h>


//Загрузка базы данных, обученной на детектирование лиц в Фас
static CvHaarClassifierCascade* cascade =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml", 0, 0, 0);

//Загрузка быза данных, обученной для детектирования глаз
static CvHaarClassifierCascade* cascade_eyes =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml", 0, 0, 0);

//Загрузка быза данных, обученной для детектирования носа
static CvHaarClassifierCascade* cascade_nose =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_nose.xml", 0, 0, 0);

//Загрузка быза данных, обученной для детектирования рота
static CvHaarClassifierCascade* cascade_mouth =
(CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_mouth.xml", 0, 0, 0);



//Запись ключевых точек в массив
void ViolaJonesDetection::writeFacePoints(CvPoint* facePoints, IplImage *imageResults, CvPoint p1, CvPoint p2, int type, int count){					//ресайз картинки	
	// Определять опатимальный вариант и брать его, сейчас берем не лучший, а первый (для носа и рта)
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
	}
	else if (type == 2
		&& p1.y >= (facePoints[2].y/* + (facePoints[6].y - facePoints[2].y) / 1.5*/)
		&& p1.y >= (facePoints[0].y + (facePoints[4].y - facePoints[0].y) / 1)){
		// рот
		facePoints[3] = p1;
		facePoints[7] = p2;
		cvRectangle(imageResults, p1, p2, CV_RGB(128, 0, 128));
	}
}

//Детектирование ключевых точек лица 
void ViolaJonesDetection::keysFaceDetect(CvHaarClassifierCascade* cscd, IplImage* imageResults, IplImage* imageFace, CvMemStorage* strg, CvPoint p, int type, CvPoint* facePoints){
	IplImage* dst = 0;
	int k;
	CvSize minSize;

	if (cscd){
		int width = imageFace->width;
		int height = imageFace->height;
		int depth = imageFace->depth;
		int nChannels = imageFace->nChannels;

		if (width < 100 || height < 100)	k = 5;
		else k = 1;

		width *= k;
		height *= k;
		dst = cvCreateImage(cvSize(width, height), depth, nChannels);
		cvResize(imageFace, dst, 1);

		minSize = cvSize(width / 4, height / 4);

		switch (type){
		case 0:
			minSize = cvSize(width / 6, height / 7);
			break;
		case 1:
			minSize = cvSize(width / 6, height / 6);
			break;
		case 2:
			minSize = cvSize(width / 5, height / 6);
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

			writeFacePoints(facePoints, imageResults, p1, p2, type, count);
			if (type == 0) count++;
			else break;

		}
	}

}

//Прорисовка соединительных линий на резулютирующем изображении
void ViolaJonesDetection::drawEvidence(IplImage *imageResults, CvPoint facePoints[8], CvPoint p1, CvPoint p2){

	bool b = 1;
	for (int i = 0; i < 8; i++)															//проверяем координаты всех точек на -1;-1
	if (facePoints[i].x < 0 || facePoints[i].y < 0)	b = 0;

	//if (b){
	CvPoint centralCoords[4];

	for (int i = 0; i < 4; i++)
		centralCoords[i] = cvPoint((facePoints[i].x + facePoints[i + 4].x) / 2, (facePoints[i].y + facePoints[i + 4].y) / 2);

	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	{

		if (centralCoords[i].x >= 0 && centralCoords[i].y >= 0 && centralCoords[j].x >= 0 && centralCoords[j].y >= 0){
			cvLine(imageResults, centralCoords[i], centralCoords[j], CV_RGB(0, 0, 255));
		}
	}
	cvRectangle(imageResults, p1, p2, CV_RGB(255, 255, 0));
	//}
	//else cvRectangle(imageResults, p1, p2, CV_RGB(255, 255, 255));
}

void ViolaJonesDetection::rotateImage(IplImage *gray_img, IplImage *small_img, CvPoint facePoints[8], CvPoint p1, CvPoint p2){
	bool b = true;
	for (int i = 0; i < 2; i++){
		if (facePoints[i].x < 0 || facePoints[i].y < 0) b = false;
		if (facePoints[i + 4].x < 0 || facePoints[i + 4].y < 0) b = false;
	}

	if (b){
		int w = small_img->width;
		int h = small_img->height;
		CvPoint pa = cvPoint((facePoints[0].x + facePoints[4].x) / 2, (facePoints[0].y + facePoints[4].y) / 2);
		CvPoint pb = cvPoint((facePoints[1].x + facePoints[5].x) / 2, (facePoints[1].y + facePoints[5].y) / 2);
		CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);

		//cvLine(resultImage, pa, pb, CV_RGB(0, 0, 255));	

		double x = (pb.x - pa.x);
		double y = (pb.y - pa.y);
		CvPoint2D32f center;

		center = cvPoint2D32f(small_img->width / 2, small_img->height / 2);

		double rad = 57.295779513;
		double angle = atan(y / x)*rad;

		cv2DRotationMatrix(center, angle, 1, transmat);

		cvWarpAffine(small_img, small_img, transmat);
	}
}


//Детектирование ключевых точек лица (вызывается из main)
void ViolaJonesDetection::cascadeDetect(IplImage* image, IplImage *imageResults, CvMemStorage* strg){
	if (cascade){
		SiftDetection *siftDetection = new SiftDetection();
		IplImage
			*gray_img = 0,
			*small_img = 0,
			*base_face = 0,
			*find_face = 0,
			*descriptors_img = 0;

		gray_img = cvCreateImage(cvGetSize(image), 8, 1);
		cvCvtColor(image, gray_img, CV_BGR2GRAY);

		CvSeq *faces = cvHaarDetectObjects(gray_img, cascade, strg, 1.2, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, cvSize(40, 50));
		for (int i = 0; i < (faces ? faces->total : 0); i++){
			CvRect* r = (CvRect*)cvGetSeqElem(faces, i);
			CvPoint p1, p2;
			CvPoint facePoints[8];

			int x = cvRound(r->x);			int y = cvRound(r->y);
			int w = cvRound(r->width);		int h = cvRound(r->height);

			p1 = cvPoint(x, y);
			p2 = cvPoint(x + w, y + h);

			small_img = cvCreateImage(cvSize(w, h), gray_img->depth, gray_img->nChannels);
			cvSetImageROI(gray_img, cvRect(x, y, w, h));
			cvCopy(gray_img, small_img, NULL);
			cvResetImageROI(gray_img);															//копируем лицо в отдельную картинку

			for (int j = 0; j < 8; j++)	facePoints[j] = cvPoint(-1, -1);						//по умолчанию координаты всех точек равны -1; -1

			keysFaceDetect(cascade_eyes, imageResults, small_img, strg, p1, 0, facePoints);
			keysFaceDetect(cascade_nose, imageResults, small_img, strg, p1, 1, facePoints);
			keysFaceDetect(cascade_mouth, imageResults, small_img, strg, p1, 2, facePoints);

			descriptors_img = small_img;
			char str[9]; sprintf(str, "%d face", i);

			//Поворот картинки
			rotateImage(gray_img, descriptors_img, facePoints, p1, p2);
			siftDetection->findDescriptors(descriptors_img, str);

			drawEvidence(imageResults, facePoints, p1, p2);

		}

		// освобождаем ресурсы
		delete siftDetection;
		cvReleaseImage(&gray_img);
		cvReleaseImage(&small_img);
		cvReleaseImage(&base_face);
		cvReleaseImage(&find_face);
	}
	else{
		cout << "cascade error" << endl;
		return;
	}
}
