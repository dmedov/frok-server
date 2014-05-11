#include "stdafx.h"
#include "common.h"
#include "ViolaJonesDetection.h"
#include "DescriptorDetection.h"
#include "EigenDetector_v2.h"
#include <fstream>
#include <vector>
#include <string>
#include <Math.h>
#include <stdlib.h>
#include "io.h"
#include "json.h"

ViolaJonesDetection::ViolaJonesDetection(){
	faceCascades = FaceCascades();
}


ViolaJonesDetection::~ViolaJonesDetection(){
	cout << "delete all" << endl;
	cvReleaseHaarClassifierCascade(&faceCascades.face);
	cvReleaseHaarClassifierCascade(&faceCascades.eyes);
	cvReleaseHaarClassifierCascade(&faceCascades.nose);
	cvReleaseHaarClassifierCascade(&faceCascades.mouth);
	cvReleaseHaarClassifierCascade(&faceCascades.eye);
	cvReleaseHaarClassifierCascade(&faceCascades.righteye2);
	cvReleaseHaarClassifierCascade(&faceCascades.lefteye2);
}

//Запись ключевых точек в массив
void ViolaJonesDetection::writeFacePoints(ImageCoordinats pointKeyFase, ImageCoordinats pointFase, int type){
	CvPoint p1 = pointKeyFase.p1;
	CvPoint p2 = pointKeyFase.p2;
	CvPoint p = pointFase.p1;
	int w = pointFase.p2.x - pointFase.p1.x;
	int h = pointFase.p2.y - pointFase.p1.y;

	CvPoint center = cvPoint((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);

	if (type == 0 && center.y <= h / 1.8 + p.y){
		//tree_glasses, если центр глаза <= w/2? то он правый, в противном случае левый
		if (center.x <= w / 2 + p.x){
			facePoints[0] = p1;	facePoints[4] = p2;
		}
		else{
			facePoints[1] = p1;	facePoints[5] = p2;
		}
	}
	else if (type == 1 && p1.y >= (facePoints[0].y)){
		// нос
		facePoints[2] = p1;	facePoints[6] = p2;
	}
	else if (type == 2
		&& p1.y >= (facePoints[2].y)
		&& p1.y >= (facePoints[0].y)
		&& center.y > h / 1.35 + p.y){
		// рот
		facePoints[3] = cvPoint(p1.x, p1.y);	facePoints[7] = cvPoint(p2.x, p2.y);
	}
	else if (type == 3
		&& center.x <= w / 2 + p.x
		&& center.y <= h / 1.8 + p.y
		){
		facePoints[0] = p1;	facePoints[4] = p2;
	}
	else if (type == 4
		&& center.x >= w / 2 + p.x
		&& center.y <= h / 1.8 + p.y
		){
		//второй глаз
		facePoints[1] = p1;	facePoints[5] = p2;
	}
}

//Прорисовка линий на резулютирующем изображении
bool ViolaJonesDetection::drawEvidence(ImageCoordinats pointFase, bool draw){
	CvPoint p1 = pointFase.p1;
	CvPoint p2 = pointFase.p2;
	int count = 0;
	for (int i = 0; i < 8; i++)															//проверяем координаты всех точек на -1;-1
	if (facePoints[i].x >= 0 && facePoints[i].y >= 0){
		count++;
	}

	if (count >= 4){
		if (draw){
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

			cvRectangle(imageResults, facePoints[0], facePoints[4], CV_RGB(0, 255, 0));
			cvRectangle(imageResults, facePoints[1], facePoints[5], CV_RGB(0, 0, 255));
			cvRectangle(imageResults, facePoints[2], facePoints[6], CV_RGB(255, 100, 255));
			cvRectangle(imageResults, facePoints[3], facePoints[7], CV_RGB(128, 0, 128));
		}
		return true;
	}
	return false;
}

int ViolaJonesDetection::defineRotate(){
	double rad = 57.295779513;
	CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);
	// Два глаза
	if ((facePoints[0].x > 0 && facePoints[0].y > 0) && (facePoints[1].x > 0 && facePoints[1].y > 0)) {

		int w = face_img->width;
		int h = face_img->height;

		CvPoint pa = cvPoint((facePoints[0].x + facePoints[4].x) / 2, (facePoints[0].y + facePoints[4].y) / 2);
		CvPoint pb = cvPoint((facePoints[1].x + facePoints[5].x) / 2, (facePoints[1].y + facePoints[5].y) / 2);

		double x = (pb.x - pa.x);
		double y = (pb.y - pa.y);
		CvPoint2D32f center;

		center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);

		double angle = atan(y / x)*rad;

		cv2DRotationMatrix(center, angle, 1, transmat);
		cvWarpAffine(face_img, face_img, transmat);
		cvReleaseMat(&transmat);
		return 0;
	}

	// Рот и нос
	else if ((facePoints[2].x > 0 && facePoints[2].y > 0) && (facePoints[3].x > 0 && facePoints[3].y > 0)){

		int w = face_img->width;
		int h = face_img->height;

		CvPoint pa = cvPoint((facePoints[2].x + facePoints[6].x) / 2, (facePoints[2].y + facePoints[6].y) / 2);
		CvPoint pb = cvPoint((facePoints[3].x + facePoints[7].x) / 2, (facePoints[3].y + facePoints[7].y) / 2);

		CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);
		double x = (pb.x - pa.x);
		double y = (pb.y - pa.y);
		CvPoint2D32f center;

		center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);
		double angle = 0;
		angle = atan(y / x)*rad;

		if (abs(angle) > 30){
			cvReleaseMat(&transmat);
			return -1;
		}
		if (angle > 0)  angle -= 90;
		else if (angle < 0) angle += 90;
		else angle = 90;

		cv2DRotationMatrix(center, angle, 1, transmat);
		cvWarpAffine(face_img, face_img, transmat);
		cvReleaseMat(&transmat);
		return 0;
	}
	return -1;
}

//Выделение лица из всего изображения с лицом и вырезаем из общей картинки
IplImage* ViolaJonesDetection::imposeMask(CvPoint p){
	int x, y, width, height, width_roi, height_roi;

	width = face_img->width;
	height = face_img->height;

	x = (int)(width / 5);
	y = (int)(height / 4);
	width_roi = width - x * 2;
	height_roi = height;

	IplImage *img = cvCreateImage(cvSize(width - x * 2, height - y), face_img->depth, face_img->nChannels);

	cvSetImageROI(face_img, cvRect(x, y, width_roi, height_roi));
	cvCopy(face_img, img, NULL);
	cvResetImageROI(face_img);															//копируем лицо в отдельную картинку

	return img;
}

void ViolaJonesDetection::createJson(DataJson dataJson){
	string outJson;
	outJson.append("{ results: [");

	int vector_size = static_cast<int>(dataJson.ids->size());

	for (int i = 0; i < vector_size; i++){
		double probability = dataJson.probs->at(i);
		int id = dataJson.ids->at(i);
		CvPoint p1 = dataJson.p1s->at(i);
		CvPoint p2 = dataJson.p2s->at(i);

		for (int j = 0; j < vector_size; j++){
			if (dataJson.ids->at(j) == id && probability < dataJson.probs->at(j)){
				dataJson.ids->at(i) = -1;
				dataJson.probs->at(i) = 0;
			}
		}
	}

	for (int i = 0; i < vector_size; i++){
		double probability = dataJson.probs->at(i);
		int id = dataJson.ids->at(i);
		CvPoint p1 = dataJson.p1s->at(i);
		CvPoint p2 = dataJson.p2s->at(i);
		char appParams[1024];
		sprintf(appParams, "{ \"id\": \"%d\", \"x1\": \"%d\", \"y1\": \"%d\", \"x2\": \"%d\", \"y2\": \"%d\", \"P\": \"%.1f\" }", id, p1.x, p1.y, p2.x, p2.y, probability);
		outJson.append(appParams);


		CvScalar textColor = CV_RGB(0, 230, 255);	// light blue text
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0, 1, CV_AA);
		char text[256];
		if (probability >= 20)
			sprintf(text, "id: %d (%.1f%%)", id, probability);
		else
			sprintf(text, "id: ?");
		cvPutText(imageResults, text, cvPoint(p1.x, p1.y - 12), &font, textColor);
	}
	outJson.append(" ] }");

	ofstream out("results.json");
	out << outJson;
	out.close();
	delete dataJson.ids;
	delete dataJson.p1s;
	delete dataJson.p2s;
	delete dataJson.probs;
}

//Детектирование ключевых точек лица 
void ViolaJonesDetection::keysFaceDetect(CvHaarClassifierCascade* cscd
	, CvPoint pointFace, int type){

	if (!cscd){
		cout << "cascade error" << endl;
		return;
	}

	IplImage* dst = 0;
	int k;
	CvSize minSize, maxSize;
	CvSeq *objects;


	int width = face_img->width;
	int height = face_img->height;
	int depth = face_img->depth;
	int nChannels = face_img->nChannels;

	if (width < 200 || height < 200)	k = 5;
	else k = 1;

	width *= k;
	height *= k;
	dst = cvCreateImage(cvSize(width, height), depth, nChannels);
	cvResize(face_img, dst, 1);

	minSize = cvSize(width / 4, height / 4);
	maxSize = cvSize(width / 2, height / 2);

	double scale_factor = 1.2;

	if (type == 0 || type == 3 || type == 4){
		minSize = cvSize(width / 6, height / 7);
		maxSize = cvSize(width / 3, height / 4);
		double scale_factor = 1.01;
	}
	else if (type == 1){
		minSize = cvSize(width / 5, height / 6);
		maxSize = cvSize((int)(width / 3.5), (int)(height / 3.5));
	}
	else if (type == 2){
		minSize = cvSize(width / 5, height / 6);
		scale_factor = 1.4;
	}

	objects = cvHaarDetectObjects(dst, cscd, strg, scale_factor, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, minSize, maxSize);

	for (int i = 0; i < (objects ? objects->total : 0); i++){
		CvRect* r = (CvRect*)cvGetSeqElem(objects, i);
		int x = cvRound(r->x) / k;
		int y = cvRound(r->y) / k;
		int w = cvRound(r->width) / k;
		int h = cvRound(r->height) / k;

		CvPoint p1 = cvPoint(x + pointFace.x, y + pointFace.y), p2 = cvPoint(x + w + pointFace.x, y + h + pointFace.y);
		ImageCoordinats pointKeyFase, pointFase;

		pointKeyFase.p1 = cvPoint(x + pointFace.x, y + pointFace.y);
		pointKeyFase.p2 = cvPoint(x + w + pointFace.x, y + h + pointFace.y);
		pointFase.p1 = pointFace;
		pointFase.p2 = cvPoint(pointFace.x + width / k, pointFace.y + height / k);


		writeFacePoints(pointKeyFase, pointFase, type);
	}

	cvReleaseImage(&dst);
	cvRelease((void**)&objects);
}

void ViolaJonesDetection::allKeysFaceDetection(CvPoint point){
	keysFaceDetect(faceCascades.eye, point, 4);				//правый общий
	keysFaceDetect(faceCascades.righteye2, point, 4);			//правый 
	keysFaceDetect(faceCascades.righteye, point, 4);			//правый альтернатива

	keysFaceDetect(faceCascades.eye, point, 3);				//левый общий
	keysFaceDetect(faceCascades.lefteye2, point, 3);			//левый 
	keysFaceDetect(faceCascades.lefteye, point, 3);			//левый альтернатива

	keysFaceDetect(faceCascades.eyes, point, 0);				//глаза в очках

	keysFaceDetect(faceCascades.nose, point, 1);				//нос
	keysFaceDetect(faceCascades.mouth, point, 2);				//рот
}

void ViolaJonesDetection::normalizateHistFace(){
	Ptr<CLAHE> clahe = createCLAHE(2, Size(8, 8));
	clahe->apply(Mat(face_img), Mat(face_img));
	cvNormalize(face_img, face_img, 10, 250, CV_MINMAX);
}

//Детектирование лица (вызывается из main)
void ViolaJonesDetection::faceDetect(IplImage *inputImage, map <string, Ptr<FaceRecognizer>> models){
	if (!faceCascades.face){
		cout << "cascade error" << endl;
		cvReleaseHaarClassifierCascade(&faceCascades.face);
		return;
	}

	DataJson dataJson = DataJson();

	image = cvCloneImage(inputImage);
	imageResults = cvCloneImage(inputImage);

	DescriptorDetection *descriptorDetection = new DescriptorDetection();
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

	gray_img = cvCreateImage(cvGetSize(image), 8, 1);
	cvCvtColor(image, gray_img, CV_BGR2GRAY);
	strg = cvCreateMemStorage(0);										//Создание хранилища памяти
	CvSeq *faces = cvHaarDetectObjects(gray_img, faceCascades.face, strg, 1.1, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, cvSize(40, 50));

	for (int i = 0; i < (faces ? faces->total : 0); i++){
		CvRect* rect = (CvRect*)cvGetSeqElem(faces, i);
		ImageCoordinats points;

		int x = cvRound(rect->x);			int y = cvRound(rect->y);
		int w = cvRound(rect->width);		int h = cvRound(rect->height);

		points.p1 = cvPoint(x, y);
		points.p2 = cvPoint(x + w, y + h);

		face_img = cvCreateImage(cvSize(w, h), gray_img->depth, gray_img->nChannels);

		cvSetImageROI(gray_img, cvRect(x, y, w, h));
		cvCopy(gray_img, face_img, NULL);
		cvResetImageROI(gray_img);									//копируем лицо в отдельную картинку//-> to introduce to function

		for (int j = 0; j < 8; j++)
			facePoints[j] = cvPoint(-1, -1);						//по умолчанию координаты всех точек равны -1; -1

		normalizateHistFace();

		allKeysFaceDetection(points.p1);

		if (drawEvidence(points, true)){
			defineRotate();
			face_img = imposeMask(points.p1);
			face_img = cvCloneImage(&(IplImage)eigenDetector_v2->MaskFace(face_img));
			IplImage *dest = cvCreateImage(cvSize(158, 190), face_img->depth, face_img->nChannels);
			cvResize(face_img, dest, 1);
			eigenDetector_v2->recognize(models, dataJson, dest);//Распознавание
			dataJson.p1s->push_back(points.p1);
			dataJson.p2s->push_back(points.p2);
			cvReleaseImage(&dest);//-> to introduce to function
		}
	}
	
	createJson(dataJson);
	cvShowImage("image", imageResults);

	// освобождаем ресурсы
	delete descriptorDetection;
	delete eigenDetector_v2;

	cvClearMemStorage(strg);
	cvReleaseImage(&face_img);
	cvReleaseImage(&gray_img);
	cvReleaseImage(&image);
	cvReleaseImage(&imageResults);
}

//Sharing on 3 gistagrams
void equalizeFace(IplImage *faceImg) {

	Mat matFaceImg = Mat(faceImg);
	int w = matFaceImg.cols;
	int h = matFaceImg.rows;
	Mat wholeFace;
	equalizeHist(matFaceImg, wholeFace);
	int midX = w / 3;

	Mat leftSide = matFaceImg(Rect(0, 0, midX, h));

	Mat midSide = matFaceImg(Rect(midX, 0, midX, h));

	Mat rightSide = matFaceImg(Rect(2 * midX, 0, midX, h));

	equalizeHist(leftSide, leftSide);
	equalizeHist(midSide, midSide);
	equalizeHist(rightSide, rightSide);

	hconcat(leftSide, midSide, leftSide);
	hconcat(leftSide, rightSide, leftSide);

	faceImg = &IplImage(leftSide);
}

//ВЫрезание изображения с лицом
int ViolaJonesDetection::cutFace(IplImage *inputImage, const char* destPath){

	if (!faceCascades.face){
		cout << "cascade error" << endl;
		cvReleaseHaarClassifierCascade(&faceCascades.face);
		return -1;
	}

	image = cvCloneImage(inputImage);

	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

	gray_img = cvCreateImage(cvGetSize(image), 8, 1);
	cvCvtColor(image, gray_img, CV_BGR2GRAY);

	//Ptr<CLAHE> clahe = createCLAHE(2, Size(8, 8));
	//clahe->apply(Mat(gray_img), Mat(gray_img));

	strg = cvCreateMemStorage(0);										//Создание хранилища памяти
	CvSeq *faces = cvHaarDetectObjects(gray_img, faceCascades.face, strg, 1.1, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, cvSize(40, 50));
	for (int i = 0; i < (faces ? faces->total : 0); i++){
		CvRect* rect = (CvRect*)cvGetSeqElem(faces, i);
		ImageCoordinats points;

		int x = cvRound(rect->x);			int y = cvRound(rect->y);
		int w = cvRound(rect->width);		int h = cvRound(rect->height);

		points.p1 = cvPoint(x, y);
		points.p2 = cvPoint(x + w, y + h);

		face_img = cvCreateImage(cvSize(w, h), gray_img->depth, gray_img->nChannels);
		cvSetImageROI(gray_img, cvRect(x, y, w, h));
		cvCopy(gray_img, face_img, NULL);
		cvResetImageROI(gray_img);									//копируем лицо в отдельную картинку

		for (int j = 0; j < 8; j++)	
			facePoints[j] = cvPoint(-1, -1);						//по умолчанию координаты всех точек равны -1; -1

		allKeysFaceDetection(points.p1);

		normalizateHistFace();

		if (drawEvidence(points, true)){
			if (defineRotate() == 0){
				face_img = imposeMask(points.p1);
				face_img = cvCloneImage(&(IplImage)eigenDetector_v2->MaskFace(face_img));

				IplImage *dest = cvCreateImage(cvSize(158, 190), face_img->depth, face_img->nChannels);
				cvResize(face_img, dest, 1);
				//equalizeFace(dist);
				if (faces->total == 1){
					if (cvSaveImage(destPath, dest))
						cout << "\t->\t person found and saved";
				}
				cvReleaseImage(&dest);
			}

		}
	}
	delete eigenDetector_v2;

	cvClearMemStorage(strg);
	cvReleaseImage(&face_img);			// освобождаем ресурсы
	cvReleaseImage(&gray_img);
	cvReleaseImage(&image);
	cvReleaseImage(&imageResults);

	return 0;
}

//Сканирование по SIFT 

void ViolaJonesDetection::scanSIFT(Mat ffDescriptors, int faceNumber){

	cerr << "Function not supported!" << endl;
	/*
	DescriptorDetection *descriptorDetection = new DescriptorDetection();
	_finddata_t result;
	char name[512];
	long done;
	IplImage *base_face = 0, *gray_face = 0;

	sprintf(name, "%s\\*.jpg", currentUserPath);

	memset(&result, 0, sizeof(result));
	done = _findfirst(name, &result);

	int max_p = 0;
	if (done != -1)
	{
		int res = 0;
		while (res == 0)
		{
			cout << result.name;
			sprintf(name, "%s\\faces\\%s", currentUserPath, result.name);
			base_face = cvLoadImage(name);

			if (!base_face) {
				cerr << "base image load error" << endl;
				return;
			}
			else {
				gray_face = cvCreateImage(cvGetSize(base_face), 8, 1);
				cvCvtColor(base_face, gray_face, CV_BGR2GRAY);
				Mat bfDescriptors = descriptorDetection->findDescriptors(base_face, result.name, false);

				if (bfDescriptors.rows > 0){
					int p = descriptorDetection->matchDescriptors(ffDescriptors, bfDescriptors);
					if (p >= max_p){
						max_p = p;
						sprintf(name, "F%d", faceNumber);
						//cvShowImage(name, base_face);
					}
				}
			}
			res = _findnext(done, &result);
		}
	}
	_findclose(done);
	cvReleaseImage(&base_face);
	cvReleaseImage(&gray_face);
	delete descriptorDetection;*/
}
