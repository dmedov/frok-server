#include "stdafx.h"
#include "EigenDetector.h"
#include <stdlib.h>
#include <io.h>
#include <Math.h>

#include "opencv2/contrib/contrib.hpp"


IplImage** faceImgArr = 0;		//array of face images
IplImage** eigenVectArr = 0;	//eigenvectors
IplImage* pAvgTrainImg = 0;
char dir[512] = "C:\\Face_detector_OK\\\tmp\\0\\faces\\";

void EigenDetector::doPCA(CvMat* eigenValMat, int nEigens){
	//set the PCA termination criterion
	int nTrainFaces = nEigens + 1;

	CvTermCriteria calcLimit = cvTermCriteria(CV_TERMCRIT_ITER, nEigens, 1);

	// Compute average image, eigenvectors (eigenfaces) and eigenvalues (ratios).
	cvCalcEigenObjects(nTrainFaces, (void*)faceImgArr, (void*)eigenVectArr,
		CV_EIGOBJ_NO_CALLBACK, 0, 0, &calcLimit,
		pAvgTrainImg, eigenValMat->data.fl);
	// Normalize the matrix of eigenvalues.
	//cvNormalize(eigenValMat, eigenValMat, 1, 0, CV_L1, 0);

	char str[60];
	for (int i = 0; i < nEigens; i++){
		sprintf(str, "%d eigen", i);
		//cvNormalize(eigenVectArr[i], eigenVectArr[i], 1, 0, CV_C);
	}
	cvNormalize(pAvgTrainImg, pAvgTrainImg, 1, 0, CV_C);
}

void EigenDetector::learn(){

	int nEigens = 0, nTrainFaces = 0;			//number of training images;
	CvMat* eigenValMat;							//eigenvalues
	CvMat* personNumTruthMat;
	CvMat* projectedTrainFaceMat;				//projected training faces	

	nTrainFaces = calcFaces(dir);
	nEigens = nTrainFaces - 1;


	faceImgArr = (IplImage**)cvAlloc(nTrainFaces*sizeof(IplImage));
	eigenVectArr = (IplImage**)cvAlloc(nEigens*sizeof(IplImage));
	eigenValMat = cvCreateMat(1, nEigens, CV_32FC1);										//allocate the eigenvalue array
	pAvgTrainImg = cvCreateImage(cvSize(158, 158), IPL_DEPTH_32F, 1);						//allocate the averaged image
	personNumTruthMat = cvCreateMat(1, nTrainFaces, CV_32SC1);

	loadBaseFace(dir, personNumTruthMat);

	for (int i = 0; i < nEigens; i++)
		eigenVectArr[i] = cvCreateImage(cvSize(158, 158), IPL_DEPTH_32F, 1);

	doPCA(eigenValMat, nEigens);

	projectedTrainFaceMat = cvCreateMat(nTrainFaces, nEigens, CV_32FC1);					//project the training images onto the PCA subspace

	for (int i = 0; i < nTrainFaces; i++){
		cvEigenDecomposite(
			faceImgArr[i],
			nEigens,
			eigenVectArr,
			0, 0,
			pAvgTrainImg,
			projectedTrainFaceMat->data.fl + i*nEigens
			);
	}

	storeTrainingData(personNumTruthMat, projectedTrainFaceMat, eigenValMat, nEigens);

	cvRelease((void**)faceImgArr);
	cvRelease((void**)eigenVectArr);
}

void EigenDetector::storeTrainingData(CvMat* personNumTruthMat, CvMat* projectedTrainFaceMat, CvMat* eigenValMat, int nEigens){

	CvFileStorage* fileStorage;
	int nTrainFaces = nEigens + 1;
	fileStorage = cvOpenFileStorage("facedata.xml", 0, CV_STORAGE_WRITE);

	cvWriteInt(fileStorage, "nEigens", nEigens);
	cvWriteInt(fileStorage, "nTrainFaces", nTrainFaces);

	cvWrite(fileStorage, "trainPersonNumMat", personNumTruthMat, cvAttrList(0, 0));
	cvReleaseMat(&personNumTruthMat);

	cvWrite(fileStorage, "eigenValMat", eigenValMat, cvAttrList(0, 0));
	cvReleaseMat(&eigenValMat);

	cvWrite(fileStorage, "projectedTrainFaceMat", projectedTrainFaceMat, cvAttrList(0, 0));
	cvReleaseMat(&projectedTrainFaceMat);

	cvWrite(fileStorage, "avgTrainImg", pAvgTrainImg, cvAttrList(0, 0));
	cvReleaseImage(&pAvgTrainImg);

	for (int i = 0; i < nEigens; i++)
	{
		char varname[200];
		sprintf(varname, "eigenVect_%d", i);
		cvWrite(fileStorage, varname, eigenVectArr[i], cvAttrList(0, 0));
	}
	cvReleaseFileStorage(&fileStorage);
}

//«агрузка изображений в массив
void EigenDetector::loadBaseFace(char* dir, CvMat* personNumTruthMat){

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

			faceImgArr[count] = cvCreateImage(cvSize(158, 158), dist->depth, dist->nChannels);
			cvResize(dist, faceImgArr[count], 1);
			cvReleaseImage(&dist);
			personNumTruthMat->data.i[count] = 0;
			cvEqualizeHist(faceImgArr[count], faceImgArr[count]);
			count++;
			res = _findnext(done, &result);
		}

		//int  truth = personNumTruthMat->data.i[0];
		//int nearest = personNumTruthMat->data.i[0];
		//cout << truth << endl;

	}
	_findclose(done);
	cvReleaseImage(&base_face);
}

int EigenDetector::calcFaces(char* dir){
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

//**********************************распознавание****************************************
void EigenDetector::recognize(IplImage* TestImg, IplImage* resultImage, CvPoint p,int id){

	CvMat* trainPersonNumMat = 0, *projectedTrainFaceMat = 0, *eigenValMat = 0;
	int nEigens = 0, nTrainFaces = 0;
	float* projectedTestFace = 0, pConfidence = 0;
	CvFileStorage* fileStorage;

	//create a file-storage interface
	fileStorage = cvOpenFileStorage("facedata.xml", 0, CV_STORAGE_READ);
	if (!fileStorage)
	{
		fprintf(stderr, "Can't open facedata.xml \n");
		return;
	}
	nEigens = cvReadIntByName(fileStorage, 0, "nEigens", 0);
	nTrainFaces = cvReadIntByName(fileStorage, 0, "nTrainFaces", 0);
	trainPersonNumMat = (CvMat*)cvReadByName(fileStorage, 0, "trainPersonNumMat", 0);
	eigenValMat = (CvMat*)cvReadByName(fileStorage, 0, "eigenValMat", 0);
	projectedTrainFaceMat = (CvMat*)cvReadByName(fileStorage, 0, "projectedTrainFaceMat", 0);
	pAvgTrainImg = (IplImage*)cvReadByName(fileStorage, 0, "avgTrainImg", 0);
	eigenVectArr = (IplImage**)cvAlloc((nTrainFaces)*sizeof(IplImage));

	for (int i = 0; i < nEigens; i++){
		char varname[200];
		sprintf(varname, "eigenVect_%d", i);
		eigenVectArr[i] = (IplImage*)cvReadByName(fileStorage, 0, varname, 0);
	}


	projectedTestFace = (float*)cvAlloc(nEigens*sizeof(float));

	
	//project the test images onto the PCA subspace
	cvEigenDecomposite(TestImg, nEigens,
		eigenVectArr, 0, 0,
		pAvgTrainImg, projectedTestFace);

	cvShowImage("avg", pAvgTrainImg);


	float prob = findNearestNeighbor(projectedTestFace, projectedTrainFaceMat, eigenValMat, nEigens, nTrainFaces, &pConfidence);

	CvScalar textColor = CV_RGB(0, 255, 255);	// light blue text
	CvFont font;
	cvInitFont(&font, 0, 1.0, 1.0, 0, 1, CV_AA);
	char text[256];
	sprintf_s(text, "id %d, p = %f %%", id, prob);
	cvPutText(resultImage, text, cvPoint(p.x, p.y - 12), &font, textColor);


	cvReleaseFileStorage(&fileStorage);
	cvReleaseMat(&trainPersonNumMat);
	cvReleaseMat(&projectedTrainFaceMat);
	cvReleaseMat(&eigenValMat);
}

float EigenDetector::findNearestNeighbor(float * projectedTestFace, CvMat* projectedTrainFaceMat, CvMat* eigenValMat, int nEigens, int nTrainFaces, float *pConfidence)
{
	float leastDistSq = FLT_MAX;
	int iNearest = 0;

	for (int iTrain = 0; iTrain < nTrainFaces; iTrain++)
	{
		float distSq = 0;

		for (int i = 0; i < nEigens; i++)
		{
			float d_i = projectedTestFace[i] - projectedTrainFaceMat->data.fl[iTrain*nEigens + i];
			float d = d_i*d_i;
			distSq += (d / eigenValMat->data.fl[i]) + d;  // Mahalanobis distance (might give better results than Eucalidean distance)
		}

		if (distSq < leastDistSq)
		{
			leastDistSq = distSq;
			iNearest = iTrain;
		}
	}


	// Return the confidence level based on the Euclidean distance,
	// so that similar images should give a confidence between 0.5 to 1.0,
	// and very different images should give a confidence between 0.0 to 0.5.
	float  prediction = (1.0f - sqrt(leastDistSq / (nTrainFaces * nEigens)) / 255.0f);

	float p = prediction * 100;
	if (p < 0) p = 0;
	cout << p << " %" << endl;

	// Return the found index.
	return p;
}