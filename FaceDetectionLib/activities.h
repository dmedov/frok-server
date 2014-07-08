#ifndef ACTIVITIES_H
#define ACTIVITIES_H
/**
* \file activities.h
* \brief This file defines actions which our application can do via NetWork.
*
*/

/** \addtogroup Activities
*  \brief List possible FaceDetectionApp's activities
*  \{
*/

/**
* \brief Max timeout of waiting untill face selects.
*/

#define CUT_TIMEOUT            (600000)

/**
* \brief Max amount of cut threads.
*/

#define MAX_THREADS_AND_CASCADES_NUM        (1)

extern FaceCascades *cascades;

/**
* \brief Folder of id's.
*/

#define ID_PATH                "/home/zda/faces/"

/**
* \brief Folder with downloading photos.
*/

#define TARGET_PATH            "/home/zda/faces/"

#pragma pack(push, 1)


/**
* \brief Context for recognizeFromModel function.
*
*/

struct ContextForRecognize{

    /**
    * \brief Remote side's socket
    */

    SOCKET sock;

    /**
    * \brief The name of the picture where faces will be recognized.
    */

    string targetImg;

    /**
    * \brief List of friends' ids.
    */

    json::Array arrFrinedsList;
};

/**
* \brief Context for generateAndTrainBase function.
*
*/

struct ContextForTrain{

    /**
    * \brief Remote side's socket
    */

    SOCKET sock;

    /**
    * \brief List of id's for which train will be called
    */

    json::Array arrIds;
};

/**
* \brief Context for getFacesFromPhoto function.
*
*/

struct ContextForGetFaces{

    /**
    * \brief Remote side's socket
    */

    SOCKET sock;

    /**
    * \brief photo owner's id
    */

    string userId;

    /**
    * \brief Name of the photo from which faces would be found.
    */

    string photoName;
};

/**
* \brief Context for saveFaceFromPhoto function.
*
*/

struct ContextForSaveFaces{

    /**
    * \brief Remote side's socket
    */

    SOCKET sock;

    /**
    * \brief photo owner's id
    */

    string userId;

    /**
    * \brief Name of the photo from which face with coorinates \a faceCoords will be cut.
    */

    string photoName;

    /**
    * \brief Coordinates of face to be cut.
    */

    int faceNumber;
};

#pragma pack(pop)

/**
* \brief Recognizing a person on the picture.
*
* Recognizing a person on the picture by using the cascades.
*
* \return Returns 1 for succes and -1 for failure.
*
* \param[in]    *pContext        See \a ContextForRecognize.
*
*/

void recognizeFromModel(void *pContext);

/**
* \brief Cutting faces and training database.
*
* Cutting faces and training database.
*
* \return Returns 1 for succes and -1 for failure.
*
* \param[in]    *pContext        See \a ContextForTrain.
*
*/

void generateAndTrainBase(void *pContext);

/**
* \brief Getting faces from photo.
*
* Selectiing faces from photo.
*
* \return Returns 1 for succes and -1 for failure.
*
* \param[in]    *pContext        See \a ContextForGetFaces.
*
*/

void getFacesFromPhoto(void *pContext);

/**
* \brief Saving recognized on the picture face.
*
* Saving recognized on the picture face.
*
* \return Returns 1 for succes and -1 if failure.
*
* \param[in]    *pContext        See \a ContextForSaveFaces.
*
*/

void saveFaceFromPhoto(void *pContext);

/** \} */
#endif // ACTIVITIES_H
