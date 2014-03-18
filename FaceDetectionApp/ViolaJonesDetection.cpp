#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "SiftDetection.h"
#include "EigenDetector.h"

#include <Math.h>
#include <stdlib.h>
#include <io.h>

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

void saveImage(IplImage *small_img){
	srand((unsigned int)time(0));
	char filename[512];
	int random = rand() + rand() * clock();
	sprintf(filename, "C:\\Face_detector_OK\\zotov_dima\\%d.jpg", random);
	cvSaveImage(filename, small_img);
}

//Запись ключевых точек в массив
void ViolaJonesDetection::writeFacePoints(CvPoint* facePoints, IplImage *imageResults, CvPoint p1, CvPoint p2, int type, int count){					//ресайз картинки	
	// Определять опатимальный вариант и брать его, сейчас берем не лучший, а первый (для носа и рта)
	if (type == 0){
		if (count == 0){
			//первый глаз
			facePoints[0] = p1;
			facePoints[4] = p2;
			//cvRectangle(imageResults, p1, p2, CV_RGB(255, 128, 0));
		}
		else if (count == 1){
			//второй глаз
			facePoints[1] = p1;
			facePoints[5] = p2;
			//cvRectangle(imageResults, p1, p2, CV_RGB(0, 128, 255));
		}
		count++;

	}
	else if (type == 1
		&& p1.y >= (facePoints[0].y/* + (facePoints[4].y - facePoints[0].y) / 1.5*/)){
		// нос
		facePoints[2] = p1;
		facePoints[6] = p2;
		//cvRectangle(imageResults, p1, p2, CV_RGB(255, 100, 255));
	}
	else if (type == 2
		&& p1.y >= (facePoints[2].y/* + (facePoints[6].y - facePoints[2].y) / 1.5*/)
		&& p1.y >= (facePoints[0].y + (facePoints[4].y - facePoints[0].y) / 1)){
		// рот
		facePoints[3] = p1;
		facePoints[7] = p2;
		//cvRectangle(imageResults, p1, p2, CV_RGB(128, 0, 128));
	}
}

//Детектирование ключевых точек лица 
void ViolaJonesDetection::keysFaceDetect(CvHaarClassifierCascade* cscd, IplImage* imageResults, IplImage* imageFace, CvMemStorage* strg, CvPoint p, int type, CvPoint* facePoints){
	IplImage* dst = 0;
	int k;
	CvSize minSize;
	CvSeq *objects;

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

		objects = cvHaarDetectObjects(dst, cscd, strg, 1.1, 2, 0 | CV_HAAR_DO_CANNY_PRUNING, minSize);

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
	cvReleaseImage(&dst);
	cvRelease((void**)&objects);
}

//Прорисовка линий на резулютирующем изображении
boolean ViolaJonesDetection::drawEvidence(IplImage *imageResults, CvPoint facePoints[8], CvPoint p1, CvPoint p2){

	int count = 0;
	for (int i = 0; i < 8; i++)															//проверяем координаты всех точек на -1;-1
	if (facePoints[i].x >= 0 && facePoints[i].y >= 0){
		count++;
	}

	if (count){
		cvRectangle(imageResults, p1, p2, CV_RGB(255, 255, 0));							//рисуем желтый квадрат, если нашли более 1 части лица
		return true;
	}
	return false;
}

//Поворот лица (по глазам)
void ViolaJonesDetection::rotateImage(IplImage *small_img, CvPoint facePoints[8], CvPoint p1, CvPoint p2){
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

		double x = (pb.x - pa.x);
		double y = (pb.y - pa.y);
		CvPoint2D32f center;

		center = cvPoint2D32f(small_img->width / 2, small_img->height / 2);

		double rad = 57.295779513;
		double angle = atan(y / x)*rad;

		cv2DRotationMatrix(center, angle, 1, transmat);
		cvWarpAffine(small_img, small_img, transmat);
		cvReleaseMat(&transmat);
	}
}

//Детектирование лица (вызывается из main)
void ViolaJonesDetection::cascadeDetect(IplImage* image, IplImage *imageResults, CvMemStorage* strg){
	if (cascade){
		SiftDetection *siftDetection = new SiftDetection();
		EigenDetector *eigenDetector = new EigenDetector();

		IplImage
			*gray_img = 0,
			*small_img = 0,
			*base_face = 0,
			*find_face = 0,
			*descriptors_img = 0;

		gray_img = cvCreateImage(cvGetSize(image), 8, 1);
		cvCvtColor(image, gray_img, CV_BGR2GRAY);
		cvEqualizeHist(gray_img, gray_img);									//Equalize Hist

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
			cvEqualizeHist(small_img, small_img);												//нормализация гистограммы

			for (int j = 0; j < 8; j++)	facePoints[j] = cvPoint(-1, -1);						//по умолчанию координаты всех точек равны -1; -1

			keysFaceDetect(cascade_eyes, imageResults, small_img, strg, p1, 0, facePoints);
			keysFaceDetect(cascade_nose, imageResults, small_img, strg, p1, 1, facePoints);
			keysFaceDetect(cascade_mouth, imageResults, small_img, strg, p1, 2, facePoints);

			descriptors_img = small_img;
			char str[9]; sprintf(str, "%d", i);

			rotateImage(small_img, facePoints, p1, p2);											//поворот картинки по линии глаз

			//Mat ffDescriptors = siftDetection->findDescriptors(small_img, str,true);
			//scanBaseFace("C:\\Face_detector_OK\\face_base\\", ffDescriptors, i);

			eigenDetector->learn();																//Обучение


			IplImage *dist = cvCreateImage(cvSize(158, 158), small_img->depth, small_img->nChannels);
			cvResize(small_img, dist, 1);
			cout << "p (" << i << ") = ";
			eigenDetector->recognize(dist, imageResults, p1, i);														//Распознавание			
			//cvShowImage(str, dist);
			cvReleaseImage(&dist);

			boolean b = drawEvidence(imageResults, facePoints, p1, p2);


		}

		// освобождаем ресурсы
		delete siftDetection;
		delete eigenDetector;

		if (small_img != NULL)
		{
			cvReleaseImage(&small_img);
			small_img = NULL;
		}
		if (base_face != NULL)
			cvReleaseImage(&base_face);
		if (find_face != NULL)
			cvReleaseImage(&find_face);
		if (gray_img != NULL)
			cvReleaseImage(&gray_img);
	}
	else{
		cout << "cascade error" << endl;
		return;
	}
}

//Сканирование по SIFT 
void ViolaJonesDetection::scanBaseFace(char *dir, Mat ffDescriptors, int faceNumber){
	SiftDetection *siftDetection = new SiftDetection();
	_finddata_t result;
	char name[512];
	long done;
	IplImage *base_face = 0, *gray_face = 0;

	sprintf(name, "%s\\*.jpg", dir);

	memset(&result, 0, sizeof(result));
	done = _findfirst(name, &result);

	int max_p = 0;
	if (done != -1)
	{
		int res = 0;
		while (res == 0){

			cout << result.name << endl;
			sprintf(name, "%s\\%s", dir, result.name);
			base_face = cvLoadImage(name);

			if (!base_face) {
				cerr << "base image load error" << endl;
				return;
			}
			else {
				gray_face = cvCreateImage(cvGetSize(base_face), 8, 1);
				cvCvtColor(base_face, gray_face, CV_BGR2GRAY);
				cvEqualizeHist(gray_face, gray_face);
				Mat bfDescriptors = siftDetection->findDescriptors(base_face, result.name, false);

				if (bfDescriptors.rows > 0){
					int p = siftDetection->matchDescriptors(ffDescriptors, bfDescriptors);
					if (p >= max_p){
						max_p = p;
						sprintf(name, "F%d", faceNumber);

						cvShowImage(name, base_face);
					}
				}
			}
			res = _findnext(done, &result);
		}
	}
	_findclose(done);
	cvReleaseImage(&base_face);
	cvReleaseImage(&gray_face);
	delete siftDetection;
}

