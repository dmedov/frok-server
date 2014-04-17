#pragma once
class EigenDetector_v2
{
public:
	string outJson;
	void learn(char* path, char* id);
	void recognize(Ptr<FaceRecognizer> model, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2, char *dir);
	Mat  MaskFace(IplImage *img);
private:
	void loadBaseFace(char* dir, vector<Mat> * images, vector<int>* labels, int id);
	double getSimilarity(const Mat A, const Mat B);
	void loadAndRecognize(char* dir, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2);
};

