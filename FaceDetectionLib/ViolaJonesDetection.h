#pragma once
class ViolaJonesDetection
{
public:
	char* dir;

	ViolaJonesDetection();
	~ViolaJonesDetection();

	void faceDetect(IplImage *inputImage, vector <Ptr<FaceRecognizer>> models);

	void rejectFace(IplImage *inputImage, char* name);

private:
	bool drawEvidence(struct ImageCoordinats pointFase, bool draw);

	void writeFacePoints(struct ImageCoordinats pointKeyFase, struct ImageCoordinats pointFase, int type);

	void keysFaceDetect(CvHaarClassifierCascade* cscd, CvPoint pointFace, int type);

	void allKeysFaceDetection(CvPoint point);

	int defineRotate();

	Mat BEImage(Mat img, Rect roi, int maxFadeDistance);

	IplImage* imposeMask(CvPoint p);

	void scanSIFT(Mat, int);
};

struct FaceCascades{
	//Загрузка базы данных, обученной на детектирование лиц в Фас
	CvHaarClassifierCascade *face = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования глаз
	CvHaarClassifierCascade *eyes = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml", 0, 0, 0);
	CvHaarClassifierCascade *righteye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_righteye.xml", 0, 0, 0);
	CvHaarClassifierCascade *lefteye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_lefteye.xml", 0, 0, 0);
	CvHaarClassifierCascade *righteye2 = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_righteye_2splits.xml", 0, 0, 0);
	CvHaarClassifierCascade *lefteye2 = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_lefteye_2splits.xml", 0, 0, 0);
	CvHaarClassifierCascade *eye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования носа
	CvHaarClassifierCascade *nose = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_nose.xml", 0, 0, 0);

	//Загрузка быза данных, обученной для детектирования рта
	CvHaarClassifierCascade *mouth = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_mouth.xml", 0, 0, 0);

};