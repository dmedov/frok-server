/**
* \file ViolaJonesDetection.h
* \brief This file defines class ViolaJonesDetection for face detection, downloading database with faces, 
* finding key points on faces and faces normalization. Includes structure with different cascades to recognize face parts.
* Counting similarity of the faces on picture.
*
*/

#pragma once

/** \addtogroup DescriptorDetection
* \brief Structure FaceCascades includse additional cascades to recognize face parts.
*	
*\param[in]		*face		Cascade to recognize a face.
*\param[in]		*eyes		Cascade to recognize the eyes on the face.
*\param[in]		*righteye	Cascade to recognize the right eye on the face if previuos cascade hasn't worked.
*\param[in]		*lefteye	Cascade to recognize the left eye on the face if cascade "eyes" hasn't worked.
*\param[in]		*righteye2	Cascade to recognize the right eye on the face if cascade "eyes" and cascade "righteye" haven't worked.
*\param[in]		*lefteye2	Cascade to recognize the left eye on the face if cascade "eyes" and cascade "lefteye" haven't worked.
*\param[in]		*eye		Cascade to detect an open eyes only.
*\param[in]		*nose		Cascade to detect a nose.
*\param[in]		*mouth		Cascade to detect a mouth.
*
*/

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

/** \class ViolaJonesDetection
* \brief This class has made for finding key points the face, downloading faces database, normalization faces and imposing mask on faces.
*
*/

class ViolaJonesDetection
{
protected:

	/**
	* \brief Memory for Cascades detection.
	*
	* *image		Downloading picture.
	* *imageResults Picture with detected faces.
	* *face_img		Pictures with selected faces.
	* *gray_img		Picture with processed face.
	*
	*/

	IplImage *image, *imageResults, *face_img, *gray_img;

	/**
	* \brief Key points of face parts: eyes, nose, mouth.
	*/

	CvPoint facePoints[8];

	/**
	* \brief Memory for using Cascades.
	*/

	CvMemStorage* strg;

public:
	ViolaJonesDetection();
	~ViolaJonesDetection();

	void allFacesDetection(IplImage *inputImage, SOCKET outSock);

	/**
	* \brief Funtion to detect a face by Haar-Cascade
	*
	* Funtion to detect a face by Haar-Cascade.
	*
	*
	* \param[in]		inputImage		Downloading picture;
	* \param[in]		&models			Models of database for every person.
	* \param[in]		outSock			Error of connecting to server.
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	*	storage = cvCreateMemStorage(0);
	*	violaJonesDetection->dir = dir_tmp;
	*	violaJonesDetection->faceDetect(img, models);
	*
	* \endcode
	*/

	void faceDetect(IplImage *inputImage, const map <string, Ptr<FaceRecognizer>> &models, SOCKET outSock = INVALID_SOCKET);

	// return -1 for failure, 0 in case of success
	/**
	* \brief Selecting a face on the picture and saving the face.
	*
	* Selecting a face on the picture from id/photos/.jpg directory and saving the face in id/faces/.jpg.
	* Argument of this fuction is structure cutFaceThreadParams which describes below.
	* This function works in a thread.
	* 
	* <b>Usage example:</b>
	*
	* \code
	*
	* CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)param->pThis->cutFaceThread, (LPVOID)param, 0, NULL);
	*
	* \endcode
	*/

	static UINT_PTR WINAPI cutFaceThread(LPVOID params);

private:

	/**
	* \brief  Selecting faces as a rectangle on the picture.
	*
	* Selecting faces as a rectangle on the picture.
	*
	*
	* \param[in]		&pointFase		Structure with coordinates of faces points;
	* \param[out]		draw			Flag to draw on picture defined faces or not.
	*
	*/

	bool drawEvidence(const ImageCoordinats &pointFase, bool draw);

	/**
	* \brief Writing key points to the array.
	*
	* Writing key points to the array.
	*
	*
	* \param[in]		&pointKeyFace	Points of left-top and right-bottom parts of faces;
	* \param[in]		&pointFace		Left-top and right-bottom points of faces;
	* \param[in]		type			Type (number) of face part: 0 - right eye, 1 - left eye, 2 - nose, 3 - mouth.
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	*   pointKeyFase.p1 = cvPoint(x + pointFace.x, y + pointFace.y);
	*	pointKeyFase.p2 = cvPoint(x + w + pointFace.x, y + h + pointFace.y);
	*	pointFase.p1 = pointFace;
	*	pointFase.p2 = cvPoint(pointFace.x + width / k, pointFace.y + height / k);
	*
	*	writeFacePoints(pointKeyFase, pointFase, type);
	*
	* \endcode
	*/

	void writeFacePoints(const ImageCoordinats &pointKeyFase, const ImageCoordinats &pointFase, int type);

	/**
	* \brief Detecting keys parts of faces.
	*
	* This function detects keys parts of faces: eyes, nose, mouth.
	*
	* \param[in]		cscd			Cascade which includes characteristics of certain part of face;
	* \param[in]		pointFace		Left-top and right-bottom points of faces;
	* \param[in]		type			Type (number) of face part: 0 - right eye, 1 - left eye, 2 - nose, 3 - mouth.
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	*   keysFaceDetect(faceCascades.righteye, point, 4);
	*   keysFaceDetect(faceCascades.lefteye, point, 3);
	*   keysFaceDetect(faceCascades.nose, point, 1);
	*   keysFaceDetect(faceCascades.mouth, point, 2);
	*
	* \endcode
	*/

	void keysFaceDetect(CvHaarClassifierCascade* cscd, CvPoint pointFace, int type);

	/**
	* \brief Finding all key points on the face.
	*
	*  Finding all key points on the face of eyes, nose and mouth.
	*
	* \param[in]	point		All key points on the recognized face.	
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	*  for (int j = 0; j < 8; j++)
	*		facePoints[j] = cvPoint(-1, -1);
	*
	*  normalizateHistFace();
	*
	*  allKeysFaceDetection(points.p1);
	*
	* \endcode
	*/

	void allKeysFaceDetection(CvPoint point);

	/**
	* \brief Face normalization.
	*
	* This function defines how to rotate recognized face.
	*
	* \Doesn't have any arguments.
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	*   defineRotate();
	*	face_img = imposeMask(points.p1);
	*	face_img = cvCloneImage(&(IplImage)eigenDetector_v2->MaskFace(face_img));
	*	IplImage *dist = cvCreateImage(cvSize(158, 190), face_img->depth, face_img->nChannels);
	*
	* \endcode
	*/

	int defineRotate();

	//Mat BEImage(Mat img, Rect roi, int maxFadeDistance);
	
	/**
	* \brief Imposes mask.
	*
	* This function imposes mask on the face to remove some noises around the bottom part of the face.
	*
	* \return Returns the picture with mask on it.
	*
	* \param[in]	p		Face points.
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	*	defineRotate();
	*	face_img = imposeMask();
	*	face_img = cvCloneImage(&(IplImage)eigenDetector_v2->MaskFace(face_img));
	*	IplImage *dist = cvCreateImage(cvSize(158, 190), face_img->depth, face_img->nChannels);
	*
	* \endcode
	*/

	IplImage* imposeMask(CvPoint p);

	//void scanSIFT(Mat, int);
		
	/**
	* \brief Function createJson.
	*
	*  It is used primarily to transmit data from our database between a server and web application.
	*
	* \param[in]	&dataJson		JSON with data of our faces.
	* \param[in]	sock			An end-point in a communication across a network of distant side.
	*
	*/

	void createJson(const DataJson &dataJson, SOCKET sock);		// [TBD] change it to smth like show on photo or send response etc

	/**
	* \brief Normalization of histogram on the face.
	*
	*  It is used primarily to transmit data from our database between a server and web application.
	*
	* \Doesn't have any arguments.	
	*
	* <b>Usage example:</b>
	*
	* \code
	*
	*  for (int j = 0; j < 8; j++)
	*		facePoints[j] = cvPoint(-1, -1);					
	*
	*  normalizateHistFace();
	*
	*  allKeysFaceDetection(points.p1);
	*
	* \endcode
	*/

	void normalizateHistFace();
};

/**
* \brief Structure cutFaceThreadParams includes input arguments for function cutFaceThread.
*
* \param[in]	inputImage		Downloading picture.
* \param[in]	destPath		Directory with cut faces.
*
*/

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


/** \} */