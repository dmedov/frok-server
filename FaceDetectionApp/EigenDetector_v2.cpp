#include "stdafx.h"
#include "EigenDetector_v2.h"
#include <stdlib.h>
#include <io.h>
#include <Math.h>

#include "opencv2/contrib/contrib.hpp"

//Загрузка изображений в массив
void EigenDetector_v2::loadBaseFace(char* dir, vector<Mat> * images, vector<int>* labels, int id){

	_finddata_t result;
	char name[512];
	long done;
	IplImage *base_face = 0;

	sprintf(name, "%s\\*.jpg", dir);
	memset(&result, 0, sizeof(result));
	done = _findfirst(name, &result);
	int count = 0;
	int max_p = 0;

	if (done != -1)
	{
		int res = 0;
		while (res == 0){
			//cout << result.name << endl;
			sprintf(name, "%s\\%s", dir, result.name);
			IplImage *dist = cvLoadImage(name, CV_LOAD_IMAGE_GRAYSCALE);
			IplImage *resize = cvCreateImage(cvSize(158, 158), dist->depth, dist->nChannels);
			cvResize(dist, resize, 1);

			images->push_back(Mat(resize, true));
			labels->push_back(id);
			count++;
			res = _findnext(done, &result);
		}
	}
	_findclose(done);
	cvReleaseImage(&base_face);
}

int EigenDetector_v2::calcFaces(char* dir){
	_finddata_t result;
	char name[512];
	long done;
	IplImage *base_face = 0;

	sprintf(name, "%s\\*.jpg", dir);
	memset(&result, 0, sizeof(result));
	done = _findfirst(name, &result);

	int nFaces = 0;
	if (done != -1)
	{
		int res = 0;
		while (res == 0){
			nFaces++;
			res = _findnext(done, &result);
		}
	}
	return nFaces;
}

Ptr<FaceRecognizer> EigenDetector_v2::learn(char* path, Ptr<FaceRecognizer> model){
	vector<Mat> images;
	vector<int> labels;
	int nTrainFaces = 0;
	char path_id[1024];
	WIN32_FIND_DATA FindFileData;
	HANDLE hf;

	sprintf(path_id, "%s//*", path);
	hf = FindFirstFile(path_id, &FindFileData);
	if (hf != INVALID_HANDLE_VALUE){
		while (FindNextFile(hf, &FindFileData) != 0){
			char* name = FindFileData.cFileName;
			if (strcmp(name, "..")){
				char path_face[1024];
				sprintf(path_face, "%s\\%s\\faces\\", path, name);
				nTrainFaces += calcFaces(path_face);
				loadBaseFace(path_face, &images, &labels, atoi(name));

			}
		}
		FindClose(hf);
	}
	model->train(images, labels);
	return model;
}

void EigenDetector_v2::recognize(Ptr<FaceRecognizer> model, IplImage* image, IplImage* resultImage, CvPoint p){
	int
		predicted_Eigen = -1;

	double predicted_confidence = 0, prob = 0, threshold = 0;

	model->predict(Mat(image, true), predicted_Eigen, predicted_confidence);

	cout << predicted_confidence << endl;

	threshold = model->getDouble("threshold");
	if (predicted_Eigen != -1)
		prob = (predicted_confidence / threshold) * 100;

	CvScalar textColor = CV_RGB(0, 230, 255);	// light blue text
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0, 1, CV_AA);
	char text[256];
	if (predicted_Eigen > 0)	
		sprintf(text, "id: %d (%.2f%%)", predicted_Eigen, prob);
	else 
		sprintf(text, "id: ? (%.2f%%)", prob);
	cvPutText(resultImage, text, cvPoint(p.x, p.y - 12), &font, textColor);

}