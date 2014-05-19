#pragma once
class EigenDetector_v2
{
public:
	bool train(const char* idPath);
	void recognize(const map <string, Ptr<FaceRecognizer>> &models, DataJson *psDataJson, IplImage* image);
	Mat  MaskFace(IplImage *img);


private:
	void loadBaseFace(const char* facesPath, vector<Mat> * images, vector<int>* labels, int id);
	double getSimilarity2(const Mat &projected_mat, const Mat &face_mat);
	double getSimilarity(const Mat &image_mat, const Mat &reconstructedFace);
	//void loadAndRecognize(const char* dir, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2);
};

