#pragma once
class EigenDetector_v2
{
public:
	Ptr<FaceRecognizer> learn(char* path, Ptr<FaceRecognizer> model);
	void recognize(Ptr<FaceRecognizer> model, IplImage* image, IplImage* resultImage, CvPoint p);
private:

	void loadBaseFace(char* dir, vector<Mat> * images, vector<int>* labels, int id);
	double getSimilarity(const Mat A, const Mat B);
};

