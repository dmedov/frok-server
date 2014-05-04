#pragma once
class ViolaJonesDetection
{
public:
	char* dir;

	ViolaJonesDetection();
	~ViolaJonesDetection();

	void cascadeDetect(IplImage *inputImage
		, vector <Ptr<FaceRecognizer>> models);

	void rejectFace(IplImage *inputImage, char* name);

private:
	bool drawEvidence(struct imageCoordinats pointFase, bool draw);

	void writeFacePoints(struct imageCoordinats pointKeyFase, struct imageCoordinats pointFase, int type);

	void keysFaceDetect(CvHaarClassifierCascade* cscd, CvPoint pointFace, int type);

	void allKeysFaceDetection(CvPoint point);

	int defineRotate();

	Mat BEImage(Mat img
		, Rect roi
		, int maxFadeDistance);

	IplImage* imposeMask(CvPoint p);

	void scanSIFT(Mat, int);
};





