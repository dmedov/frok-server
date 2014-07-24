#include <unistd.h>
#include "FaceUserModel.h"

#define MODULE_NAME         "FACE_MODEL"

FaceUserModel::FaceUserModel(std::string userId, EnumFaceRecognizer recognizer)
{\
    this->userId = userId;
    modelType = recognizer;
    switch(modelType)
    {
    case RECOGNIZER_EIGENFACES:
    {
        model = cv::createEigenFaceRecognizer();
        break;
    }
    case RECOGNIZER_FISHER:
    default:
    {
        TRACE_F("Recognizer %x is not supported", recognizer);
        throw FROK_RESULT_INVALID_PARAMETER;
        break;
    }
    }
}

FrokResult FaceUserModel::GenerateUserModel(const char *kappaFacesPath)
{
    TRACE_T("started");
    if(kappaFacesPath == NULL)
    {
        TRACE_F_T("Invalid parameter: kappaFacesPath = %p", kappaFacesPath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    std::vector< std::string > photos;
    if(-1 == getFilesFromDir(kappaFacesPath, photos))
    {
        TRACE_F_T("Failed to get photos from directory %s", kappaFacesPath);
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    if(photos.empty())
    {
        TRACE_W_T("No photos found in directory %s. Continue...", kappaFacesPath);
    }
    else
    {
        labels.clear();
        userKappaFaces.clear();

        for(std::vector<std::string>::const_iterator it = photos.begin(); it != photos.end(); ++it)
        {
            std::string imageName = (std::string)*it;
            cv::Mat newFace;
            try
            {
                newFace = cv::imread(imageName.c_str(), CV_LOAD_IMAGE_UNCHANGED);
            }
            catch(...)
            {

                TRACE_F_T("Failed to load image %s", imageName.c_str());
                continue;
            }
            labels.push_back(userId);
            userKappaFaces.push_back(newFace);
        }

        if(!userKappaFaces.empty())
        {
            TRACE_T("Model training started");
            try
            {
                model->train(userKappaFaces, labels);
            }
            catch(...)
            {
                TRACE_F_T("Opencv failed to train model");
                return FROK_RESULT_OPENCV_ERROR;
            }
            TRACE_S_T("Model training succeed");
        }
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}
FrokResult FaceUserModel::GenerateUserModel(std::vector<cv::Mat> kappaFaces)
{
    TRACE_T("started");
    if(kappaFaces.empty())
    {
        TRACE_F_T("Invalid parameter: input vector is empty");
        return FROK_RESULT_INVALID_PARAMETER;
    }

    labels.clear();
    userKappaFaces.clear();
    userKappaFaces = kappaFaces;

    for(unsigned i = 0; i < kappaFaces.size(); i++)
    {
        labels.push_back(userId);
    }

    TRACE_T("Model training started");
    try
    {
        model->train(userKappaFaces, labels);
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to train model");
        return FROK_RESULT_OPENCV_ERROR;
    }
    TRACE_S_T("Model training succeed");
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceUserModel::AddFaceToModel(cv::Mat kappaFace)
{
    TRACE_T("started");
    switch(modelType)
    {
    case RECOGNIZER_EIGENFACES:
    {
        labels.push_back(userId);
        userKappaFaces.push_back(kappaFace);
        TRACE_T("Updating model started");
        try
        {
            model->update(userKappaFaces, labels);
        }
        catch(...)
        {
            TRACE_F_T("Opencv failed to update model");
            return FROK_RESULT_OPENCV_ERROR;
        }
        TRACE_S_T("Updating model succeed");
        break;
    }
    case RECOGNIZER_FISHER:
    default:
    {
        TRACE_F_T("Recognizer %x is not supported", modelType);
        return FROK_RESULT_INVALID_PARAMETER;
        break;
    }
    }
    TRACE_T("finished");
}

FrokResult FaceUserModel::LoadUserModel(const char *userPath)
{
    TRACE_T("started");
    if(userPath == NULL)
    {
        TRACE_F_T("Invalid parameter: userPath = %p", userPath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    std::string modelPath(userPath);
    switch(modelType)
    {
    case RECOGNIZER_EIGENFACES:
    {
        modelPath.append(USER_MODEL_FILENAME_EIGENFACES);
    }
    case RECOGNIZER_FISHER:
    default:
    {
        TRACE_F_T("Recognizer %x is not supported", modelType);
        return FROK_RESULT_INVALID_PARAMETER;
        break;
    }
    }

    if(-1 == access(modelPath.c_str(), R_OK))
    {
        TRACE_W_T("File %s not found or doesn't have read permissions", modelPath.c_str());
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    TRACE_T("Trying to load user model");
    try
    {
        model->load(modelPath.c_str());
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to load model");
        return FROK_RESULT_OPENCV_ERROR;
    }
    TRACE_S_T("Loading succeed");

    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceUserModel::SaveUserModel(const char *userPath)
{
    TRACE_T("started");
    if(userPath == NULL)
    {
        TRACE_F_T("Invalid parameter: userPath = %p", userPath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    std::string modelPath(userPath);
    switch(modelType)
    {
    case RECOGNIZER_EIGENFACES:
    {
        modelPath.append(USER_MODEL_FILENAME_EIGENFACES);
    }
    case RECOGNIZER_FISHER:
    default:
    {
        TRACE_F_T("Recognizer %x is not supported", modelType);
        return FROK_RESULT_INVALID_PARAMETER;
        break;
    }
    }

    if(0 == access(modelPath.c_str(), F_OK))
    {
        if(-1 == access(modelPath.c_str(), W_OK))
        {
            TRACE_W_T("File %s exists but doesn't have wright permissions", modelPath.c_str());
            return FROK_RESULT_UNSPECIFIED_ERROR;
        }
    }

    TRACE_T("Trying to save user model");
    try
    {
        model->save(modelPath.c_str());
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to save model");
        return FROK_RESULT_OPENCV_ERROR;
    }
    TRACE_S_T("Saving succeed");

    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}
