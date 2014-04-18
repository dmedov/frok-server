#pragma once
class EigenDetector_v2
{
public:
	void learn(char* path, char* id);
	void recognize(vector <Ptr<FaceRecognizer>> models, vector<int> *ids, vector<CvPoint> *p1s, vector<CvPoint> *p2s, vector<double> *probs, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2, char *dir);
	Mat  MaskFace(IplImage *img);
private:
	void loadBaseFace(char* dir, vector<Mat> * images, vector<int>* labels, int id);
	double getSimilarity(const Mat dif);
	void loadAndRecognize(char* dir, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2);
};

