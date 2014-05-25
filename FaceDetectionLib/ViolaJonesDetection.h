/**
* \file ViolaJonesDetection.h
* \brief This file defines class ViolaJonesDetection for face detection, downloading database with faces,
* finding key points on faces and faces normalization. Includes structure with different cascades to recognize face parts.
* Counting similarity of the faces on picture.
*
*/

#pragma once

/** \addtogroup DescriptorDetection
* \brief Structure that includes additional cascades to recognize face parts.
*/

struct FaceCascades{

	/**
	* \brief Cascade to recognize a face..
	*/
	CvHaarClassifierCascade *face = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml", 0, 0, 0);

	/**
	* \brief Cascade to recognize the eyes on the face.
	*/

	CvHaarClassifierCascade *eyes = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml", 0, 0, 0);

	/**
	* \brief Cascade to recognize the right eye on the face if previuos cascade hasn't worked.
	*/

	CvHaarClassifierCascade *righteye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_righteye.xml", 0, 0, 0);

	/**
	* \brief Cascade to recognize the left eye on the face if cascade "eyes" hasn't worked.
	*/

	CvHaarClassifierCascade *lefteye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_lefteye.xml", 0, 0, 0);

	/**
	* \brief Cascade to recognize the right eye on the face if cascade "eyes" and cascade "righteye" haven't worked.
	*/

	CvHaarClassifierCascade *righteye2 = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_righteye_2splits.xml", 0, 0, 0);

	/**
	* \brief Cascade to recognize the left eye on the face if cascade "eyes" and cascade "lefteye" haven't worked.
	*/

	CvHaarClassifierCascade *lefteye2 = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_lefteye_2splits.xml", 0, 0, 0);

	/**
	* \brief Cascade to detect an open eyes only.
	*/

	CvHaarClassifierCascade *eye = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye.xml", 0, 0, 0);

	/**
	* \brief Cascade to detect a nose.
	*/

	CvHaarClassifierCascade *nose = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_nose.xml", 0, 0, 0);

	/**
	* \brief Cascade to detect a mouth.
	*/

	CvHaarClassifierCascade *mouth = (CvHaarClassifierCascade*)cvLoad("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_mcs_mouth.xml", 0, 0, 0);
};

/**
* \brief Object of structure FaceCascades for using cascdes to recognize parts of the face.
*
*/

extern FaceCascades faceCascades;

/**
* \brief faceDetectionCS is used to provide access to shared resources.
*
*/

extern CRITICAL_SECTION faceDetectionCS;

/** \class ViolaJonesDetection
* \brief This class has made for finding key points the face, downloading faces database, normalization faces and imposing mask on faces.
*
*/

class ViolaJonesDetection
{
protected:

	/**
	* \brief Common objects of image.
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

	/**
	* \brief Constructor of ViolaJonesDetection class.
	*
	* Creating the object of the ViolaJonesDetection class and initializing fields of class.
	*
	*/
	ViolaJonesDetection();

	/**
	* \brief Destructor of ViolaJonesDetection class.
	*
	*  Used to "clean up" when an object is no longer necessary.
	*
	*/
	~ViolaJonesDetection();

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

	/**
	* \brief Selecting a face on the picture and saving the face.
	*
	* Selecting a face on the picture from id/photos/ and saving the face in id/faces/.
	* Argument of this fuction is structure cutFaceThreadParams which describes below.
	* This functions should be called in a new thread. Return expected_event_type of this event.
	*
	* \return -1 for failure, 0 in for success.
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
	* \param[in]		draw			Flag to draw on picture defined faces or not.
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
	* \brief Selecting face.
	*
	* This function finds a face on the picture and selects the face.
	*
	* \return Returns the picture with face.
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
	*  It is used primarily to transmit data of recognizing through specified socket.
	*
	* \param[in]	&dataJson		JSON with data of our faces.
	* \param[in]	sock			An end-point in a communication across a network of distant side.
	*
	*/

	void createJson(const DataJson &dataJson, SOCKET sock);		// [TBD] change it to smth like show on photo or send response etc

	/**
	* \brief Normalization of histogram on the face.
	*
	*  Normalization of histogram on the face to aviod some noises.
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
*/

struct cutFaceThreadParams
{
	/**
	* \brief Constructor of structure cutFaceThreadParams.
	*
	* Creating the object of the cutFaceThreadParams structure and initializing fields of structure.
	*
	* \param[in]	inputImage			The picture from which we select faces.
	* \param[in]	destPath			Directory with selected faces.
	*
	*/

	cutFaceThreadParams(IplImage *inputImage, const char* destPath)
	{
		this->inputImage = new IplImage;
		this->inputImage = inputImage;
		this->destPath = new char[strlen(destPath)];
		strcpy(this->destPath, destPath);
		pThis = new ViolaJonesDetection;
	}

	/**
	* \brief Destructor of ViolaJonesDetection class.
	*
	*  Used to "clean up" when an object is no longer necessary.
	*
	*/

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