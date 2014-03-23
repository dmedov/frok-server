#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "SiftDetection.h"
#include "EigenDetector.h"
#include "EigenDetector_v2.h"
#include <vector>
#include <string>
#include <Math.h>
#include <stdlib.h>
#include <io.h>

static CvHaarClassifierCascade* cascade, *cascade_eyes, *cascade_nose, *cascade_mouth;


ViolaJonesDetection::ViolaJonesDetection(){
	//Загрузка базы данных, обученной на детектирование лиц в Фас
	cascade = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования глаз
	cascade_eyes = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования носа
	cascade_nose = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_nose.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования рта
	cascade_mouth = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_mouth.xml", 0, 0, 0);
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
		facePoints[2] = p1; //cvPoint(p1.x,p1.y);
		facePoints[6] = p2; //cvPoint(p2.x, p2.y);
		//cvRectangle(imageResults, p1, p2, CV_RGB(255, 100, 255));
	}
	else if (type == 2
		&& p1.y >= (facePoints[2].y/* + (facePoints[6].y - facePoints[2].y) / 1.5*/)
		&& p1.y >= (facePoints[0].y + (facePoints[4].y - facePoints[0].y) / 1)){
		// рот
		facePoints[3] = cvPoint(p1.x, p1.y);
		facePoints[7] = cvPoint(p2.x, p2.y);
		//cvRectangle(imageResults, p1, p2, CV_RGB(128, 0, 128));
	}
}

//Детектирование ключевых точек лица 
void ViolaJonesDetection::keysFaceDetect(CvHaarClassifierCascade* cscd, IplImage* imageResults, IplImage* imageFace, CvMemStorage* strg, CvPoint p, int type, CvPoint* facePoints){
	if (!cscd){
		cout << "cascade error" << endl;
		return;
	}


	IplImage* dst = 0;
	int k;
	CvSize minSize;
	CvSeq *objects;


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
		//cvRectangle(imageResults, p1, p2, CV_RGB(255, 255, 0));							//рисуем желтый квадрат, если нашли более 1 части лица
		int w = (p2.x - p1.x);
		int h = (p2.y - p1.y);

		cvLine(imageResults, p1, cvPoint(p1.x + w / 4, p1.y), CV_RGB(128, 128, 255));
		cvLine(imageResults, p1, cvPoint(p1.x, p1.y + h / 4), CV_RGB(128, 128, 255));
		cvLine(imageResults, p2, cvPoint(p2.x - w / 4, p2.y), CV_RGB(128, 128, 255));
		cvLine(imageResults, p2, cvPoint(p2.x, p2.y - h / 4), CV_RGB(128, 128, 255));

		cvLine(imageResults, cvPoint(p1.x + w, p1.y), cvPoint(p1.x + w - w / 4, p1.y), CV_RGB(128, 128, 255));
		cvLine(imageResults, cvPoint(p1.x + w, p1.y), cvPoint(p1.x + w, p1.y + h / 4), CV_RGB(128, 128, 255));
		cvLine(imageResults, cvPoint(p2.x - w, p2.y), cvPoint(p2.x - w, p2.y - h / 4), CV_RGB(128, 128, 255));
		cvLine(imageResults, cvPoint(p2.x - w, p2.y), cvPoint(p2.x - w + w / 4, p2.y), CV_RGB(128, 128, 255));

		return true;
	}
	return false;
}

void ViolaJonesDetection::rotateImage(IplImage *gray_img, IplImage *small_img, CvPoint facePoints[8], CvPoint p1, CvPoint p2){

	double rad = 57.295779513;
	bool b = true;
	bool n = false;

	for (int i = 0; i < 2; i++){
		if (facePoints[i].x < 0 || facePoints[i].y < 0) b = false;
		if (facePoints[i + 4].x < 0 || facePoints[i + 4].y < 0) b = false;
	}


	/*for (int i = 0; i < 2; i++){
		if (facePoints[i+2].x < 0 || facePoints[i+2].y < 0) n = false;
		if (facePoints[i+5].x < 0 || facePoints[i+5].y < 0) n = false;
		}
		*/

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

		double angle = atan(y / x)*rad;

		cv2DRotationMatrix(center, angle, 1, transmat);
		cvWarpAffine(small_img, small_img, transmat);
		cvReleaseMat(&transmat);

	}
	else {
		int w = small_img->width;
		int h = small_img->height;
		CvPoint pa = cvPoint((facePoints[2].x + facePoints[6].x) / 2, (facePoints[2].y + facePoints[6].y) / 2);
		CvPoint pb = cvPoint((facePoints[3].x + facePoints[7].x) / 2, (facePoints[3].y + facePoints[7].y) / 2);
		CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);

		cvLine(gray_img, pa, pb, CV_RGB(250, 0, 0), 1, 8);
		//cvShowImage("David_Duhovniy", gray_img);

		double x = (pb.x - pa.x);
		double y = (pb.y - pa.y);
		CvPoint2D32f center;

		center = cvPoint2D32f(small_img->width / 2, small_img->height / 2);

		double angle = atan(y / x)*rad;

		angle -= 90;
		angle /= 2;


		cv2DRotationMatrix(center, angle, 1, transmat);

		cvWarpAffine(small_img, small_img, transmat);
		cvReleaseMat(&transmat);
	}

}

void ViolaJonesDetection::BEImage(cv::Mat _img, cv::Rect _roi, int _maxFadeDistance) {
	cv::imshow("BEIMAGE", _img);
	cv::Mat fadeMask = cv::Mat(_img.size(), CV_32FC1, cv::Scalar(1));
	cv::rectangle(fadeMask, _roi, cv::Scalar(0), -1);

	/*cv::Mat fadeMask = cv::Mat(_img.size(), CV_8UC1, cv::Scalar(0, 0, 0, 0));
	cv::rectangle(fadeMask, _roi, cv::Scalar(255, 255, 255, 255), -1);*/


	cv::imshow("mask", fadeMask);

	cv::Mat dt;
	cv::distanceTransform(fadeMask > 0, dt, CV_DIST_L2, CV_DIST_MASK_PRECISE);



	// fade to a maximum distance:
	double maxFadeDist;

	if (_maxFadeDistance > 0)
		maxFadeDist = _maxFadeDistance;
	else
	{
		// find min/max vals
		double min, max;
		cv::minMaxLoc(dt, &min, &max);
		maxFadeDist = max;
	}


	//dt = 1.0-(dt* 1.0 / max);   // values between 0 and 1 since min val should alwaysbe 0
	dt = 1.0-(dt* 1.0 / maxFadeDist); // values between 0 and 1 in fading region

	cv::imshow("blending mask", dt);


	cv::Mat imgF;
	_img.convertTo(imgF, CV_32FC3);


	std::vector<cv::Mat> channels;
	cv::split(imgF, channels);
	// multiply pixel value with the quality weights for image 1
	for (unsigned int i = 0; i<channels.size(); ++i)
		channels[i] = channels[i].mul(dt);

	cv::Mat outF;
	cv::merge(channels, outF);

	cv::Mat out;
	outF.convertTo(out, CV_8UC3);
	cv::imshow("result BEImage", out);
}

//Детектирование лица (вызывается из main)
void ViolaJonesDetection::cascadeDetect(IplImage* image, IplImage *imageResults, IplImage *ret_img, CvMemStorage* strg, Ptr<FaceRecognizer> model){
	if (!cascade){
		cout << "cascade error" << endl;
		cvReleaseHaarClassifierCascade(&cascade);
		return;
	}

	SiftDetection *siftDetection = new SiftDetection();
	EigenDetector *eigenDetector = new EigenDetector();
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

	IplImage
		*gray_img = 0,
		*small_img = 0,
		*base_face = 0,
		*find_face = 0,
		*descriptors_img = 0;

	gray_img = cvCreateImage(cvGetSize(image), 8, 1);
	cvCvtColor(image, gray_img, CV_BGR2GRAY);
	//cvEqualizeHist(gray_img, gray_img);									//Equalize Hist

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
		cout << w << " " << h;
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

		rotateImage(gray_img, small_img, facePoints, p1, p2);											//поворот картинки по линии глаз
		Mat in = Mat(small_img);
		BEImage(in, cv::Rect(in.cols / 4, in.rows / 4, in.cols / 2, in.rows / 2), in.cols / 8);
		IplImage *dist = cvCreateImage(cvSize(158, 158), small_img->depth, small_img->nChannels);
		cvResize(small_img, dist, 1);
		//eigenDetector->recognize(dist, imageResults, p1, i);														//Распознавание	
		
		eigenDetector_v2->recognize(model, dist, imageResults, p1);
		
		cvReleaseImage(&dist);
		boolean b = drawEvidence(imageResults, facePoints, p1, p2);
	}

	// освобождаем ресурсы
	delete siftDetection;
	delete eigenDetector;
	delete eigenDetector_v2;

	if (small_img != NULL)
		cvReleaseImage(&small_img);
	if (base_face != NULL)
		cvReleaseImage(&base_face);
	if (find_face != NULL)
		cvReleaseImage(&find_face);
	if (gray_img != NULL)
		cvReleaseImage(&gray_img);
}

//Сканирование по SIFT 
void ViolaJonesDetection::scanSIFT(char *dir, Mat ffDescriptors, int faceNumber){
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
				//cvEqualizeHist(gray_face, gray_face);
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

ViolaJonesDetection::~ViolaJonesDetection(){
	cout << "delete all" << endl;
	cvReleaseHaarClassifierCascade(&cascade);
	cvReleaseHaarClassifierCascade(&cascade_eyes);
	cvReleaseHaarClassifierCascade(&cascade_nose);
	cvReleaseHaarClassifierCascade(&cascade_mouth);
}
