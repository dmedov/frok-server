#pragma once
class EigenDetector_v2
{
public:
	void learn(char* path, char* id);
	void recognize(vector <Ptr<FaceRecognizer>> models, struct DataJson dataJson, IplImage* image, char *dir);
	Mat  MaskFace(IplImage *img);


private:
	void loadBaseFace(char* dir, vector<Mat> * images, vector<int>* labels, int id);
	void loadAndRecognize(char* dir, IplImage* image, IplImage* resultImage, CvPoint p1, CvPoint p2);
};

