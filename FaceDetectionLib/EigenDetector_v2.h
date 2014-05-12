#pragma once
class EigenDetector_v2
{
public:
	void train(const char* idPath);
	void recognize(map <string, Ptr<FaceRecognizer>> models, struct DataJson dataJson, IplImage* image);
	Mat  MaskFace(IplImage *img);


private:
	void loadBaseFace(const char* facesPath, vector<Mat> * images, vector<int>* labels, int id);
	//void loadAndRecognize(const char* dir, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2);
};

