#pragma once

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

extern FaceCascades faceCascades;
extern CRITICAL_SECTION faceDetectionCS;
class ViolaJonesDetection
{
protected:
	IplImage *image, *imageResults, *face_img, *gray_img;
	CvPoint facePoints[8];
	CvMemStorage* strg;
public:
	ViolaJonesDetection();
	~ViolaJonesDetection();

	void faceDetect(IplImage *inputImage, map <string, Ptr<FaceRecognizer>> models, SOCKET outSock = INVALID_SOCKET);

	// return -1 for failure, 0 in case of success
	static DWORD WINAPI cutFaceThread(LPVOID params);

private:
	bool drawEvidence(struct ImageCoordinats pointFase, bool draw);

	void writeFacePoints(struct ImageCoordinats pointKeyFase, struct ImageCoordinats pointFase, int type);

	void keysFaceDetect(CvHaarClassifierCascade* cscd, CvPoint pointFace, int type);

	void allKeysFaceDetection(CvPoint point);

	int defineRotate();

	Mat BEImage(Mat img, Rect roi, int maxFadeDistance);

	IplImage* imposeMask(CvPoint p);

	void scanSIFT(Mat, int);

	void createJson(DataJson dataJson, SOCKET sock);		// [TBD] change it to smth like show on photo or send response etc
	void normalizateHistFace();
};

struct cutFaceThreadParams
{
	cutFaceThreadParams(IplImage *inputImage, const char* destPath)
	{
		this->inputImage = new IplImage;
		this->inputImage = inputImage;
		this->destPath = new char[strlen(destPath)];
		strcpy(this->destPath, destPath);
		pThis = new ViolaJonesDetection;
	}

	~cutFaceThreadParams()
	{
		cvReleaseImage(&inputImage);
		delete pThis;
	}
	IplImage *inputImage;
	char* destPath;
	ViolaJonesDetection *pThis;
};