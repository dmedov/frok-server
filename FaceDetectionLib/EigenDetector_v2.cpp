#include "stdafx.h"
#include "EigenDetector_v2.h"
#include "DescriptorDetection.h"
#include <stdlib.h>
#include <io.h>
#include <Math.h>

#include "opencv2/contrib/contrib.hpp"




/*
	Загрузка изображений в массив
	dir - ./faces, из которой берутся изображения с лицом для обучения
	*/
void EigenDetector_v2::loadBaseFace(char* dir, vector<Mat> * images, vector<int>* labels, int id){

	_finddata_t result;
	char name[1024];
	long done;
	IplImage *base_face = 0;

	sprintf(name, "%s\\*.jpg", dir);
	memset(&result, 0, sizeof(result));
	done = _findfirst(name, &result);

	if (done != -1)
	{
		int res = 0;
		while (res == 0){
			sprintf(name, "%s\\%s", dir, result.name);
			IplImage *dist = cvLoadImage(name, CV_LOAD_IMAGE_GRAYSCALE);
			IplImage *resize = cvCreateImage(cvSize(158, 190), dist->depth, dist->nChannels);
			cvResize(dist, resize, 1);

			images->push_back(Mat(resize, true));
			labels->push_back(id);

			cout << name << endl;
			res = _findnext(done, &result);
		}
	}
	_findclose(done);
	cvReleaseImage(&base_face);
}

static Mat norm_0_255(InputArray _src) {
	Mat src = _src.getMat();
	// Create and return normalized image:
	Mat dst;
	switch (src.channels()) {
	case 1:
		cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
		break;
	case 3:
		cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
		break;
	default:
		src.copyTo(dst);
		break;
	}
	return dst;
}

/*
	Обучение FaceRecognizer по базе
	model - модель FaceRecognizer, сохраняется после обучение в yml
	path - путь до базы людей(папка для каждого человека названная его id, содержит в себе ./faces и ./photos)
	*/
void EigenDetector_v2::learn(char* path, char* id){

	char path_id[1024];
	WIN32_FIND_DATA FindFileData;
	HANDLE hf;

	sprintf(path_id, "%s//*", path);
	hf = FindFirstFile(path_id, &FindFileData);
	if (hf != INVALID_HANDLE_VALUE){
		while (FindNextFile(hf, &FindFileData) != 0){
			char* name = FindFileData.cFileName;
			if (strcmp(name, "..") && ((!strcmp(name, id)) || (!strcmp(id, "-1")))){
				vector<Mat> images;
				vector<int> labels;
				Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
				char path_face[1024];
				char path_eigen[1024];
				sprintf(path_face, "%s\\%s\\faces\\", path, name);
				loadBaseFace(path_face, &images, &labels, atoi(name));
				model->train(images, labels);

				sprintf(path_eigen, "%s\\%s\\%s", path, name, "eigenface.yml");
				model->save(path_eigen);
				cout << path_eigen << " has been saved.\n" << endl;

			}
		}
		FindClose(hf);
	}
}

Mat EigenDetector_v2::MaskFace(IplImage *img) {
	Mat _img = Mat(img, true);
	int x = _img.cols;
	int y = _img.rows;
	Mat mask = Mat(_img.size(), CV_8UC1, Scalar(255));
	Point faceCenter = Point(cvRound(x * 0.5),
		cvRound(y * 0.25));
	Size size = Size(cvRound(x * 0.55), cvRound(y * 0.8));
	ellipse(mask, faceCenter, size, 0, 0, 360, Scalar(0),
		CV_FILLED);
	// Apply the elliptical mask on the face, to remove corners.
	// Sets corners to gray, without touching the inner face.

	_img.setTo(Scalar(128), mask);

	return _img;
}

double EigenDetector_v2::getSimilarity(const Mat A, const Mat B) {

	Mat dif = abs(A - B);
	int koef = 0;
	double err = 0;
	for (int y(0); y < dif.rows; ++y){
		for (int x(0); x < dif.cols; ++x){

			int d = dif.at<unsigned char>(y, x);
			if (d <= 200)
				koef += d;
			/*if (d > 100 && d < 150) koef++;*/
		}
	}

	err = (double)koef / (dif.cols*dif.rows * 32);

	//cout << err << " " << koef << endl;
	if (err > 1) err = 1;
	return (1 - err);

}

// сравнение объектов по моментам их контуров 
double testMatch(IplImage* image, IplImage* dif){
	assert(image != 0);
	assert(dif != 0);

	IplImage* binI = cvCreateImage(cvGetSize(image), 8, 1);
	IplImage* binT = cvCreateImage(cvGetSize(dif), 8, 1);

	// получаем границы изображения и шаблона
	cvCanny(image, binI, 10, 300, 3);
	cvCanny(dif, binT, 10, 300, 3);

	Mat image_mat = Mat(binI, true);
	Mat dif_mat = Mat(binT, true);
	int koef_image = 0, koef_dif = 0;;
	for (int y(0); y < image_mat.rows; ++y){
		for (int x(0); x < image_mat.cols; ++x){
			if (image_mat.at<unsigned char>(y, x) > 0)	koef_image++;
			if (dif_mat.at<unsigned char>(y, x) > 0)	koef_dif++;
		}
	}

	cvShowImage("binI", binI);
	cvShowImage("binT", binT);

	//cvWaitKey(0);

	cvReleaseImage(&binI);
	cvReleaseImage(&binT);
	double probability = ((double)koef_image / (double)koef_dif) / 1.7;
	if (probability >= 1) probability = 0.99;

	return probability;
}

double getSimilarity2(const Mat A, const Mat B) {
	// Calculate the L2 relative error between the 2 images.
	double errorL2 = norm(A, B, CV_L2);
	// Scale the value since L2 is summed across all pixels.
	double similarity = errorL2 / (double)(A.rows * A.cols);
	return similarity / 1.7;
}


void EigenDetector_v2::recognize(vector <Ptr<FaceRecognizer>> models, vector<int> *ids, vector<CvPoint> *p1s, vector<CvPoint> *p2s, vector<double> *probs, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2, char *dir){

	double old_prob = 0;

	char path_id[1024];
	char path_yml[1024];
	char result_name[512] = "-1";
	WIN32_FIND_DATA FindFileData;
	HANDLE hf;

	Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
	int i = 0;
	sprintf(path_id, "%s//*", dir);
	hf = FindFirstFile(path_id, &FindFileData);
	if (hf != INVALID_HANDLE_VALUE){
		while (FindNextFile(hf, &FindFileData) != 0){
			char* name = FindFileData.cFileName;
			if (strcmp(name, "..")){
				sprintf(path_yml, "%s\\%s\\eigenface.yml", dir, name);

				//models.pop_back->load(path_yml);
				model = models[i];
				i++;
				double prob = 0;

				Mat image_mat = Mat(image, true);

				// Get some required data from the FaceRecognizer model.
				Mat eigenvectors = model->get<Mat>("eigenvectors");
				Mat averageFaceRow = model->get<Mat>("mean");
				// Project the input image onto the eigenspace.
				Mat projection = subspaceProject(eigenvectors, averageFaceRow, image_mat.reshape(1, 1));
				// Generate the reconstructed face back from the eigenspace.
				Mat reconstructionRow = subspaceReconstruct(eigenvectors, averageFaceRow, projection);
				// Make it a rectangular shaped image instead of a single row.
				Mat reconstructionMat = reconstructionRow.reshape(1, image->height);
				// Convert the floating-point pixels to regular 8-bit uchar.
				Mat reconstructedFace = Mat(reconstructionMat.size(), CV_8U);
				reconstructionMat.convertTo(reconstructedFace, CV_8U, 1, 0);

				Mat dif = abs(image_mat - reconstructedFace);

				cout << name << " ";
				testMatch(image, &(IplImage)dif);


				prob = (testMatch(image, &(IplImage)reconstructedFace) + getSimilarity(image_mat, reconstructedFace) + getSimilarity2(image_mat, reconstructedFace)) / 2;

				if (prob > old_prob){
					char dig[1024];
					sprintf(dig, "repr %d", p1.x + p1.y);
					imshow(dig, reconstructedFace);
					sprintf(dig, "face %d", p1.x + p1.y);
					imshow(dig, image_mat);
					sprintf(dig, "diff %d", p1.x + p1.y);
					imshow(dig, dif);
					old_prob = prob;
					sprintf(result_name, "%s", name);
				}

			}
		}
		FindClose(hf);
	}

	ids->push_back(atoi(result_name));
	probs->push_back(old_prob * 100);
	p1s->push_back(p1);
	p2s->push_back(p2);

}