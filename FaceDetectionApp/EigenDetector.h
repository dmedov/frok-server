#pragma once
class EigenDetector
{
public:
	void learn();
	void recognize(IplImage* TestImg);
private:
	int calcFaces(char* dir);

	int loadFaceImgArray(char* filename);

	void doPCA(CvMat* eigenValMat
		, int nEigens
		); 	//do PCA on the training faces

	void storeTrainingData(
		CvMat* personNumTruthMat
		, CvMat* projectedTrainFaceMat
		, CvMat* eigenValMat
		, int nEigens
		);

	void recognize(
		IplImage* TestImg
		, CvMat* projectedTrainFaceMat
		, CvMat* eigenValMat
		, int nEigens
		);

	int findNearestNeighbor(float * projectedTestFace
		, CvMat* projectedTrainFaceMat
		, CvMat* eigenValMat
		, int nEigens
		, int nTrainFaces
		, float *pConfidence
		);

	void loadBaseFace(char* dir, CvMat* personNumTruthMat);
};