#pragma once
class ViolaJonesDetection
{
public:
	void cascadeDetect(IplImage*, IplImage *, CvMemStorage*);
private:
	void writeFacePoints(CvPoint*, IplImage *, CvPoint, CvPoint, int, int);
	void keysFaceDetect(CvHaarClassifierCascade*, IplImage*, IplImage*, CvMemStorage*, CvPoint, int, CvPoint*);
	void drawEvidence(IplImage *, CvPoint[], CvPoint, CvPoint);
	void rotateImage(IplImage *, IplImage *, CvPoint[], CvPoint, CvPoint);
	void rejectImage(IplImage *, IplImage *, int, int, int, int);
};


