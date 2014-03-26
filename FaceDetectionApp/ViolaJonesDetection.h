#pragma once
class ViolaJonesDetection
{
public:
	ViolaJonesDetection();
	~ViolaJonesDetection();
	void cascadeDetect(IplImage* image, IplImage *imageResults, CvMemStorage* strg, Ptr<FaceRecognizer> model);
	void rejectFace(IplImage* image, CvMemStorage* strg, char* dir, char* name);
private:
	bool drawEvidence(IplImage *imageResults, CvPoint facePoints[8], CvPoint p1, CvPoint p2, bool draw);
	void writeFacePoints(CvPoint*, IplImage *, CvPoint, CvPoint, int, int);
	void keysFaceDetect(CvHaarClassifierCascade*, IplImage*, IplImage*, CvMemStorage*, CvPoint, int, CvPoint*);
	void rotateImage(IplImage *, IplImage *, CvPoint[], CvPoint, CvPoint);
	Mat BEImage(cv::Mat _img, cv::Rect _roi, int _maxFadeDistance);
	IplImage* imposeMask(IplImage *small_img);
	void scanSIFT(char *dir, Mat, int);
	void saveImageRandom(IplImage *face, char* dir);
};


