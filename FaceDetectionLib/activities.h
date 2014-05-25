/**
* \file activities.h
* \brief This file defines actions which our application can do via NetWork.
*
*/

/** \addtogroup Activities.
*  \brief List of possible actions of our applications:
*  \{
*/

#pragma once

/**
* \brief Max time of waiting untill faces select.
*/

#define CUT_TIMEOUT			(600000)

/**
* \brief Max amount of cut threads.
*/

#define MAX_THREADS_NUM		(10)


/**
* \brief Folder of id's.
*/

//#define ID_PATH				"Z:\\HerFace\\Faces\\"
#define ID_PATH				"Z:\\frok\\"

/**
* \brief Folder with downloading photos.
*/

//#define TARGET_PATH			"Z:\\HerFace\\Faces\\"
#define TARGET_PATH			"Z:\\frok\\1\\"

#pragma pack(push, 1)


/**
* \brief This Structure includes context for function Recognize in class EigenDetector_v2.
*
*/

struct ContextForRecognize{

	/**
	* \brief An end-point in a communication across a network of distant side.
	*/

	SOCKET sock;

	/**
	* \brief The name of the picture where faces should be recognized.
	*/

	string targetImg;

	/**
	* \brief List of id's of friends.
	*/

	json::Array arrFrinedsList;
};

/**
* \brief This Structure includes context for function Train in class EigenDetector_v2.
*
*/

struct ContextForTrain{

	/**
	* \brief An end-point in a communication across a network of distant side.
	*/

	SOCKET sock;

	/**
	* \brief List of id's on which we are going to train our application.
	*/

	json::Array arrIds;
};

/**
* \brief This Structure includes context to get faces.
*
*/

struct ContextForGetFaces{

	/**
	* \brief An end-point in a communication across a network of distant side.
	*/

	SOCKET sock;

	/**
	* \brief Getting user id.
	*/

	string userId;

	/**
	* \brief Name of the photo from which we are going to get faces.
	*/

	string photoName;
};

/**
* \brief This Structure includes context to get faces.
*
*/

struct ContextForSaveFaces{

	/**
	* \brief An end-point in a communication across a network of distant side.
	*/

	SOCKET sock;

	/**
	* \brief Getting user id.
	*/

	string userId;

	/**
	* \brief Name of the photo with recognized face.
	*/

	string photoName;

	/**
	* \brief List of faces coordinates.
	*/

	json::Object faceCoords;
};

#pragma pack(pop)

/**
* 
*/

/**
* \brief Recognizing a person on the picture.
*
* Recognizing a person on the picture by using cascades.
*
* \return Returns 1 for succes and -1 if failure.
*
* \param[in]	*pContext		Have a look for more information at /a ContextForRecognize.
*
*/

DWORD recognizeFromModel(void *pContext);

/**
* \brief Generating and training database with faces.
*
* Generating and training database by faces recognized on the picture.
*
* \return Returns 1 for succes and -1 if failure.
*
* \param[in]	*pContext		Have a look for more information at /a ContextForTrain.
*
*/

DWORD generateAndTrainBase(void *pContext);

/**
* \brief Getting faces from photo.
*
* Selectiing faces on the picture with many faces.
*
* \return Returns 1 for succes and -1 if failure.
*
* \param[in]	*pContext		Have a look for more information at /a ContextForGetFaces.
*
*/

DWORD getFacesFromPhoto(void *pContext);

/**
* \brief Saving recognized faces on the picture.
*
* Saving recognized faces on the picture.
*
* \return Returns 1 for succes and -1 if failure.
*
* \param[in]	*pContext		Have a look for more information at /a ContextForSaveFaces.
*
*/

DWORD saveFaceFromPhoto(void *pContext);


/** \} */