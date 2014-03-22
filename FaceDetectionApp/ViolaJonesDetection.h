#pragma once
class ViolaJonesDetection
{
public:
	ViolaJonesDetection();
	~ViolaJonesDetection();
	void cascadeDetect(IplImage* image, IplImage *imageResults, IplImage *ret_img, CvMemStorage* strg, Ptr<FaceRecognizer> model);
private:
	boolean drawEvidence(IplImage *, CvPoint[], CvPoint, CvPoint);
	void writeFacePoints(CvPoint*, IplImage *, CvPoint, CvPoint, int, int);
	void keysFaceDetect(CvHaarClassifierCascade*, IplImage*, IplImage*, CvMemStorage*, CvPoint, int, CvPoint*);	
	void rotateImage(IplImage *, IplImage *, CvPoint[], CvPoint, CvPoint);
	void scanSIFT(char *dir, Mat, int);
};


