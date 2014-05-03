#include "stdafx.h"
#include "EigenDetector_v2.h"
#include "DescriptorDetection.h"
#include <stdlib.h>
#include <io.h>
#include <Math.h>
#include <algorithm>

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

double getSimilarity(const Mat image_mat, const Mat reconstructedFace) {

	IplImage *blr_img = cvCloneImage(&(IplImage)image_mat);
	IplImage *blr_rec = cvCloneImage(&(IplImage)reconstructedFace);
	cvErode(blr_img, blr_img, 0, 0);
	cvErode(blr_rec, blr_rec, 0, 0);

	Mat dif = abs(Mat(blr_img) - Mat(blr_rec));

	int koef = 0;
	double err = 0;
	for (int y(0); y < dif.rows; ++y){
		for (int x(0); x < dif.cols; ++x){
			int d = dif.at<unsigned char>(y, x);
			if (d >= 40)
			koef += d;
		}
	}
	err = (double)koef / (dif.cols*dif.rows * 40);



	double prob = (1 - err);

	if (prob > 1) prob = 0.99;
	if (prob < 0) prob = 0.01;


	return prob;

}

double getSimilarity2(const Mat projected_mat, const Mat face_mat) {

	CvSize imagesSize = cvSize(projected_mat.cols, projected_mat.rows);
	IplImage *projectedStorage = cvCreateImage(imagesSize, IPL_DEPTH_32F, 1);
	IplImage *faceSorage = cvCreateImage(imagesSize, IPL_DEPTH_32F, 1);

	cvCornerMinEigenVal(&(IplImage)projected_mat, projectedStorage, 15, 7);
	cvCornerMinEigenVal(&(IplImage)face_mat, faceSorage, 15, 7);

	Mat dif_mat = abs(Mat(projectedStorage) - Mat(faceSorage));
	//imshow("dif", dif);


	IplImage *dif_img = &(IplImage)dif_mat;

	double err = 0;
	for (int y = 0; y < dif_mat.rows; ++y){
		for (int x = 0; x < dif_mat.cols; ++x){
			err += cvGet2D(dif_img, y, x).val[0];
		}
	}


	err /= (dif_mat.rows*dif_mat.cols);
	imshow("dif", dif_mat);
	imshow("rep", projected_mat);
	imshow("img", face_mat);

	cvReleaseImage(&projectedStorage);
	cvReleaseImage(&faceSorage);

	err *= 2.5;
	double prob = (1 - err);
	if (prob > 1) prob = 0.99;
	if (prob < 0) prob = 0.01;
	prob -= 0.65;
	prob *= 3;

	return prob;
}
// сравнение объектов по моментам их контуров 
double testMatch(IplImage* image, IplImage* rec){
	assert(image != 0);
	assert(rec != 0);

	IplImage* binI = cvCreateImage(cvGetSize(image), 8, 1);
	IplImage* binT = cvCreateImage(cvGetSize(rec), 8, 1);

	// получаем границы изображения и шаблона
	cvCanny(image, binI, 10, 200, 3);
	cvCanny(rec, binT, 10, 300, 3);

	Mat image_mat = Mat(binI, true);
	Mat dif_mat = Mat(binT, true);


	// для хранения контуров
	CvMemStorage* storage1 = cvCreateMemStorage(0);
	CvMemStorage* storage2 = cvCreateMemStorage(0);
	CvSeq* contoursI = 0, *contoursT = 0;

	// находим контуры изображения
	int contoursCont = cvFindContours(binI, storage1, &contoursI, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

	// находим контуры шаблона
	int contoursCont2 = cvFindContours(binT, storage2, &contoursT, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));


	CvSeq* seqT = 0;
	double perimT = 0;

	if (contoursT != 0){
		// находим самый длинный контур 
		for (CvSeq* seq0 = contoursT; seq0 != 0; seq0 = seq0->h_next){
			double perim = cvContourPerimeter(seq0);
			if (perim > perimT){
				perimT = perim;
				seqT = seq0;
			}
		}
	}

	double matchM = 0;
	// обходим контуры изображения 
	int counter = 0;
	if (contoursI != 0 && contoursT != 0){
		// поиск лучшего совпадения контуров по их моментам 
		for (CvSeq* seq0 = contoursI; seq0 != 0; seq0 = seq0->h_next){
			double match0 = cvMatchShapes(seq0, seqT, CV_CONTOURS_MATCH_I3);
			if (match0 > matchM){
				matchM = match0;
			}
		}
	}

	cvShowImage("binI", binI);
	cvShowImage("binT", binT);


	cvReleaseImage(&binI);
	cvReleaseImage(&binT);
	cout << matchM << endl;
	/*double probability = ((double)koef_image / (double)koef_dif) / 1.7;
	if (probability >= 1) probability = 0.99;*/

	return 0;
}

__int64 calcImageHash(IplImage* src, bool show_results)
{
	if (!src){
		return 0;
	}

	IplImage *res = 0, *bin = 0;

	res = cvCreateImage(cvSize(8, 8), src->depth, src->nChannels);
	bin = cvCreateImage(cvSize(8, 8), IPL_DEPTH_8U, 1);

	// уменьшаем картинку
	cvResize(src, res);

	// вычисляем среднее
	CvScalar average = cvAvg(res);
	// получим бинарное изображение относительно среднего
	// для этого воспользуемся пороговым преобразованием
	cvThreshold(res, bin, average.val[0], 255, CV_THRESH_BINARY);

	// построим хэш
	__int64 hash = 0;

	int i = 0;
	// пробегаемся по всем пикселям изображения
	for (int y = 0; y < bin->height; y++) {
		uchar* ptr = (uchar*)(bin->imageData + y * bin->widthStep);
		for (int x = 0; x < bin->width; x++) {
			// 1 канал
			if (ptr[x]){
				// hash |= 1<<i;  // warning C4334: '<<' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
				hash |= 1i64 << i;
			}
			i++;
		}
	}

	// освобождаем ресурсы
	cvReleaseImage(&res);
	cvReleaseImage(&bin);

	return hash;
}

__int64 calcHammingDistance(__int64 x, __int64 y)
{
	__int64 dist = 0, val = x ^ y;

	// Count the number of set bits
	while (val)
	{
		++dist;
		val &= val - 1;
	}

	return dist;
}


void EigenDetector_v2::recognize(vector <Ptr<FaceRecognizer>> models, vector<int> *ids, vector<CvPoint> *p1s, vector<CvPoint> *p2s, vector<double> *probs, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2, char *dir){

	double old_prob = 0;

	char path_id[1024];
	char path_yml[1024];
	char result_name[512] = "-1";
	WIN32_FIND_DATA FindFileData;
	HANDLE hf;

	Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
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

				__int64 hashO = calcImageHash(&(IplImage)reconstructedFace, true);
				__int64 hashI = calcImageHash(image, false);
				__int64 dist = calcHammingDistance(hashO, hashI);
				if (dist <= 11){  // если хэш больше 8, то вероятность -> 0

					double prob2 = getSimilarity2(reconstructedFace, image_mat);
					double prob1 = getSimilarity(reconstructedFace, image_mat);

					prob = max(prob2, prob1)/2;
					

					cout << name << " " << prob1 << "\t" << prob2 << "\t" << prob << endl;

					if (prob > old_prob){
						char dig[1024];
						sprintf(dig, "repr %d", p1.x + p1.y);
						//imshow(dig, reconstructedFace);
						sprintf(dig, "face %d", p1.x + p1.y);
						//imshow(dig, image_mat);

						old_prob = prob;
						sprintf(result_name, "%s", name);

					}
					cvWaitKey(0);
				}


			}
		}
		FindClose(hf);
	}
	cout << endl;

	ids->push_back(atoi(result_name));
	probs->push_back(old_prob * 100);
	p1s->push_back(p1);
	p2s->push_back(p2);

}