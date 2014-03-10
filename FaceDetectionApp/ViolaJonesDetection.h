#pragma once
class ViolaJonesDetection
{
public:
	void cascadeDetect(IplImage*, IplImage *, CvMemStorage*);
private:
	boolean drawEvidence(IplImage *, CvPoint[], CvPoint, CvPoint);
	void writeFacePoints(CvPoint*, IplImage *, CvPoint, CvPoint, int, int);
	void keysFaceDetect(CvHaarClassifierCascade*, IplImage*, IplImage*, CvMemStorage*, CvPoint, int, CvPoint*);	
	void rotateImage(IplImage *, CvPoint[], CvPoint, CvPoint);
	void rejectImage(IplImage *, IplImage *, int, int, int, int);
	void scanBaseFace(char *dir, Mat, int);
};


