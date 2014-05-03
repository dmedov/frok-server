#include "stdafx.h"
#include "ViolaJonesDetection.h"
#include "DescriptorDetection.h"
#include "EigenDetector_v2.h"
#include <fstream>
#include <vector>
#include <string>
#include <Math.h>
#include <stdlib.h>
#include <io.h>



static CvHaarClassifierCascade* cascade, *cascade_eyes, *cascade_eye, *cascade_righteye, *cascade_lefteye, *cascade_righteye2, *cascade_lefteye2, *cascade_nose, *cascade_mouth;
void equalizeFace(IplImage *faceImg);

ViolaJonesDetection::ViolaJonesDetection(){
	//Загрузка базы данных, обученной на детектирование лиц в Фас
	cascade = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования глаз
	cascade_eyes = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml", 0, 0, 0);
	cascade_righteye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_righteye.xml", 0, 0, 0);
	cascade_lefteye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_lefteye.xml", 0, 0, 0);
	cascade_righteye2 = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_righteye_2splits.xml", 0, 0, 0);
	cascade_lefteye2 = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_lefteye_2splits.xml", 0, 0, 0);
	cascade_eye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования носа
	cascade_nose = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_nose.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования рта
	cascade_mouth = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_mouth.xml", 0, 0, 0);
}

//Запись ключевых точек в массив
void ViolaJonesDetection::writeFacePoints(CvPoint* facePoints, IplImage *imageResults, CvPoint p1, CvPoint p2, CvPoint p, int w, int h, int type){					//ресайз картинки	
	// Определять опатимальный вариант и брать его, сейчас берем не лучший, а первый (для носа и рта)

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

//Детектирование ключевых точек лица 
void ViolaJonesDetection::keysFaceDetect(CvHaarClassifierCascade* cscd
	, IplImage* imageResults
	, IplImage* imageFace
	, CvMemStorage* strg
	, CvPoint p, int type
	, CvPoint* facePoints){

	if (!cscd){
		cout << "cascade error" << endl;
		return;
	}

	IplImage* dst = 0;
	int k;
	CvSize minSize, maxSize;
	CvSeq *objects;


	int width = imageFace->width;
	int height = imageFace->height;
	int depth = imageFace->depth;
	int nChannels = imageFace->nChannels;

	if (width < 200 || height < 200)	k = 5;
	else k = 1;

	width *= k;
	height *= k;
	dst = cvCreateImage(cvSize(width, height), depth, nChannels);
	cvResize(imageFace, dst, 1);

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

		CvPoint p1 = cvPoint(x + p.x, y + p.y), p2 = cvPoint(x + w + p.x, y + h + p.y);
		writeFacePoints(facePoints, imageResults, p1, p2, p, width / k, height / k, type);
	}

	cvReleaseImage(&dst);
	cvRelease((void**)&objects);
}

//Прорисовка линий на резулютирующем изображении
bool ViolaJonesDetection::drawEvidence(IplImage *imageResults, CvPoint* facePoints, CvPoint p1, CvPoint p2, bool draw){

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


int defineRotate(IplImage *gray_img, IplImage *small_img, CvPoint facePoints[], CvPoint p1, CvPoint p2){
	double rad = 57.295779513;
	CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);
	// Два глаза
	if ((facePoints[0].x > 0 && facePoints[0].y > 0) && (facePoints[1].x > 0 && facePoints[1].y > 0)) {

		int w = small_img->width;
		int h = small_img->height;

		CvPoint pa = cvPoint((facePoints[0].x + facePoints[4].x) / 2, (facePoints[0].y + facePoints[4].y) / 2);
		CvPoint pb = cvPoint((facePoints[1].x + facePoints[5].x) / 2, (facePoints[1].y + facePoints[5].y) / 2);



		double x = (pb.x - pa.x);
		double y = (pb.y - pa.y);
		CvPoint2D32f center;

		center = cvPoint2D32f(small_img->width / 2, small_img->height / 2);

		double angle = atan(y / x)*rad;

		cv2DRotationMatrix(center, angle, 1, transmat);
		cvWarpAffine(small_img, small_img, transmat);
		cvReleaseMat(&transmat);
		return 0;
	}

	// Рот и нос
	else if ((facePoints[2].x > 0 && facePoints[2].y > 0) && (facePoints[3].x > 0 && facePoints[3].y > 0)){

		int w = small_img->width;
		int h = small_img->height;

		CvPoint pa = cvPoint((facePoints[2].x + facePoints[6].x) / 2, (facePoints[2].y + facePoints[6].y) / 2);
		CvPoint pb = cvPoint((facePoints[3].x + facePoints[7].x) / 2, (facePoints[3].y + facePoints[7].y) / 2);

		CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);
		double x = (pb.x - pa.x);
		double y = (pb.y - pa.y);
		CvPoint2D32f center;

		center = cvPoint2D32f(small_img->width / 2, small_img->height / 2);
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
		cvWarpAffine(small_img, small_img, transmat);
		cvReleaseMat(&transmat);
		return 0;
	}
	return -1;
}

//Выделение лица из всего изображения с лицом и вырезаем из общей картинки
IplImage* ViolaJonesDetection::imposeMask(IplImage *small_img, IplImage*gray_img, CvPoint p){
	int x, y, width, height, width_roi, height_roi;

	width = small_img->width;
	height = small_img->height;

	x = (int)(width / 5);
	y = (int)(height / 4);
	width_roi = width - x * 2;
	height_roi = height;

	IplImage *img = cvCreateImage(cvSize(width - x * 2, height - y), small_img->depth, small_img->nChannels);

	cvSetImageROI(small_img, cvRect(x, y, width_roi, height_roi));
	cvCopy(small_img, img, NULL);
	cvResetImageROI(small_img);															//копируем лицо в отдельную картинку

	return img;
}

//Детектирование лица (вызывается из main)
void ViolaJonesDetection::cascadeDetect(IplImage* image, IplImage *imageResults, CvMemStorage* strg, vector <Ptr<FaceRecognizer>> models, char* dir){
	if (!cascade){
		cout << "cascade error" << endl;
		cvReleaseHaarClassifierCascade(&cascade);
		return;
	}

	DescriptorDetection *descriptorDetection = new DescriptorDetection();
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

	vector<int> *ids = new vector<int>;
	vector<CvPoint> *p1s = new vector<CvPoint>;
	vector<CvPoint> *p2s = new vector<CvPoint>;
	vector<double>*probs = new vector<double>;

	IplImage
		*gray_img = 0,
		*small_img = 0;

	string outJson;

	gray_img = cvCreateImage(cvGetSize(image), 8, 1);
	cvCvtColor(image, gray_img, CV_BGR2GRAY);

	CvSeq *faces = cvHaarDetectObjects(gray_img, cascade, strg, 1.1, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, cvSize(40, 50));

	outJson.append("{ results: [");

	for (int i = 0; i < (faces ? faces->total : 0); i++){
		CvRect* rect = (CvRect*)cvGetSeqElem(faces, i);
		CvPoint p1, p2;
		CvPoint facePoints[8];

		int x = cvRound(rect->x);			int y = cvRound(rect->y);
		int w = cvRound(rect->width);		int h = cvRound(rect->height);

		p1 = cvPoint(x, y);
		p2 = cvPoint(x + w, y + h);

		small_img = cvCreateImage(cvSize(w, h), gray_img->depth, gray_img->nChannels);

		cvSetImageROI(gray_img, cvRect(x, y, w, h));
		cvCopy(gray_img, small_img, NULL);
		cvResetImageROI(gray_img);															//копируем лицо в отдельную картинку


		for (int j = 0; j < 8; j++)	facePoints[j] = cvPoint(-1, -1);						//по умолчанию координаты всех точек равны -1; -1

		Ptr<CLAHE> clahe = createCLAHE(2, Size(8, 8));
		clahe->apply(Mat(small_img), Mat(small_img));
		cvNormalize(small_img, small_img, 10, 250, CV_MINMAX);


		keysFaceDetect(cascade_eye, image, small_img, strg, p1, 4, facePoints);				//правый общий
		keysFaceDetect(cascade_righteye2, image, small_img, strg, p1, 4, facePoints);		//правый 
		keysFaceDetect(cascade_righteye, image, small_img, strg, p1, 4, facePoints);		//правый альтернатива

		keysFaceDetect(cascade_eye, image, small_img, strg, p1, 3, facePoints);				//левый общий
		keysFaceDetect(cascade_lefteye2, image, small_img, strg, p1, 3, facePoints);		//левый 
		keysFaceDetect(cascade_lefteye, image, small_img, strg, p1, 3, facePoints);			//левый альтернатива

		keysFaceDetect(cascade_eyes, image, small_img, strg, p1, 0, facePoints);			//глаза в очках

		keysFaceDetect(cascade_nose, image, small_img, strg, p1, 1, facePoints);			//нос
		keysFaceDetect(cascade_mouth, image, small_img, strg, p1, 2, facePoints);			//рот

		
		char str[9]; sprintf(str, "%d", i);
		if (drawEvidence(imageResults, facePoints, p1, p2, true)){
			defineRotate(gray_img, small_img, facePoints, p1, p2);
			small_img = imposeMask(small_img, gray_img, p1);
			small_img = cvCloneImage(&(IplImage)eigenDetector_v2->MaskFace(small_img));
			IplImage *dist = cvCreateImage(cvSize(158, 190), small_img->depth, small_img->nChannels);
			cvResize(small_img, dist, 1);
			eigenDetector_v2->recognize(models, ids, p1s, p2s, probs, dist, imageResults, p1, p2, dir);//Распознавание
			cvReleaseImage(&dist);

		}
		
	}

	int vector_size = static_cast<int>(ids->size());

	for (int i = 0; i < vector_size; i++){
		double probability = probs->at(i);
		int id = ids->at(i);
		CvPoint p1 = p1s->at(i);
		CvPoint p2 = p2s->at(i);

		for (int j = 0; j < vector_size; j++){
			if (ids->at(j) == id && probability < probs->at(j)){
				ids->at(i) = -1;
				probs->at(i) = 0;
			}
		}
	}

	for (int i = 0; i < vector_size; i++){
		double probability = probs->at(i);
		int id = ids->at(i);
		CvPoint p1 = p1s->at(i);
		CvPoint p2 = p2s->at(i);
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


	// освобождаем ресурсы
	delete descriptorDetection;
	delete eigenDetector_v2;
	delete ids;
	delete p1s;
	delete p2s;
	delete probs;

	cvReleaseImage(&small_img);
	cvReleaseImage(&gray_img);
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
void ViolaJonesDetection::rejectFace(IplImage* image, CvMemStorage* strg, char* dir, char* name){

	if (!cascade){
		cout << "cascade error" << endl;
		cvReleaseHaarClassifierCascade(&cascade);
		return;
	}
	EigenDetector_v2 *eigenDetector_v2 = new EigenDetector_v2();

	IplImage
		*gray_img = 0,
		*small_img = 0;

	gray_img = cvCreateImage(cvGetSize(image), 8, 1);
	cvCvtColor(image, gray_img, CV_BGR2GRAY);
	//cvEqualizeHist(gray_img, gray_img);
	Ptr<CLAHE> clahe = createCLAHE(2, Size(8, 8));
	clahe->apply(Mat(gray_img), Mat(gray_img));

	CvSeq *faces = cvHaarDetectObjects(gray_img, cascade, strg, 1.1, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, cvSize(40, 50));
	for (int i = 0; i < (faces ? faces->total : 0); i++){
		CvRect* rect = (CvRect*)cvGetSeqElem(faces, i);
		CvPoint p1, p2;
		CvPoint facePoints[8];

		int x = cvRound(rect->x);			int y = cvRound(rect->y);
		int w = cvRound(rect->width);		int h = cvRound(rect->height);

		p1 = cvPoint(x, y);
		p2 = cvPoint(x + w, y + h);

		small_img = cvCreateImage(cvSize(w, h), gray_img->depth, gray_img->nChannels);
		cvSetImageROI(gray_img, cvRect(x, y, w, h));
		cvCopy(gray_img, small_img, NULL);
		cvResetImageROI(gray_img);															//копируем лицо в отдельную картинку

		for (int j = 0; j < 8; j++)	facePoints[j] = cvPoint(-1, -1);						//по умолчанию координаты всех точек равны -1; -1

		keysFaceDetect(cascade_lefteye2, image, small_img, strg, p1, 3, facePoints);		//правый 
		keysFaceDetect(cascade_righteye, image, small_img, strg, p1, 3, facePoints);		//правый альтернатива
		keysFaceDetect(cascade_eye, image, small_img, strg, p1, 3, facePoints);				//правый альтернатива


		keysFaceDetect(cascade_righteye2, image, small_img, strg, p1, 4, facePoints);		//левый 
		keysFaceDetect(cascade_lefteye, image, small_img, strg, p1, 4, facePoints);			//левый альтернатива
		keysFaceDetect(cascade_eye, image, small_img, strg, p1, 4, facePoints);				//правый альтернатива

		keysFaceDetect(cascade_eyes, image, small_img, strg, p1, 0, facePoints);			//глаза в очках

		keysFaceDetect(cascade_nose, image, small_img, strg, p1, 1, facePoints);			//нос
		keysFaceDetect(cascade_mouth, image, small_img, strg, p1, 2, facePoints);			//рот

		if (drawEvidence(image, facePoints, p1, p2, true)){
			if (defineRotate(gray_img, small_img, facePoints, p1, p2) == 0){
				small_img = imposeMask(small_img, gray_img, p1);
				small_img = cvCloneImage(&(IplImage)eigenDetector_v2->MaskFace(small_img));

				IplImage *dist = cvCreateImage(cvSize(158, 190), small_img->depth, small_img->nChannels);
				cvResize(small_img, dist, 1);
				//equalizeFace(dist);
				if (faces->total == 1){
					char save_dir[1024];
					sprintf(save_dir, "%s\\faces\\%s", dir, name);
					if (cvSaveImage(save_dir, dist))
						cout << "\t->\t person found and saved";
				}
				cvReleaseImage(&dist);
			}

		}
	}
	delete eigenDetector_v2;
	cvReleaseImage(&small_img);			// освобождаем ресурсы
	cvReleaseImage(&gray_img);
}

//Сканирование по SIFT 
void ViolaJonesDetection::scanSIFT(char *dir, Mat ffDescriptors, int faceNumber){
	DescriptorDetection *descriptorDetection = new DescriptorDetection();
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

			cout << result.name;
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
	delete descriptorDetection;
}

ViolaJonesDetection::~ViolaJonesDetection(){
	cout << "delete all" << endl;
	cvReleaseHaarClassifierCascade(&cascade);
	cvReleaseHaarClassifierCascade(&cascade_eyes);
	cvReleaseHaarClassifierCascade(&cascade_nose);
	cvReleaseHaarClassifierCascade(&cascade_mouth);
	cvReleaseHaarClassifierCascade(&cascade_eye);
	cvReleaseHaarClassifierCascade(&cascade_righteye2);
	cvReleaseHaarClassifierCascade(&cascade_lefteye2);
}
