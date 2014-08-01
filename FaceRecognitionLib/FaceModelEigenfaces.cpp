#include <unistd.h>
#include "FaceModelEigenfaces.h"

#pragma GCC poison IplImage
#define MODULE_NAME         "MODEL_EIGENFACES"

FaceModelEigenfaces::FaceModelEigenfaces()
{
    TRACE_F_T("Do not use this constructor");
}
FaceModelEigenfaces::~FaceModelEigenfaces()
{
    TRACE("~FaceModelEigenfaces");
}
FaceModelEigenfaces::FaceModelEigenfaces(std::string userId) : FaceModelAbstract(userId)
{
    model = cv::createEigenFaceRecognizer();
    TRACE("new FaceModelEigenfaces");
}

FrokResult FaceModelEigenfaces::GenerateUserModel(const char *grayFacesPath)
{
    TRACE_T("started");
    if(grayFacesPath == NULL)
    {
        TRACE_F_T("Invalid parameter: grayFacesPath = %p", grayFacesPath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    std::vector< std::string > photos;
    if(-1 == getFilesFromDir(grayFacesPath, photos))
    {
        TRACE_F_T("Failed to get photos from directory %s", grayFacesPath);
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }

    if(photos.empty())
    {
        TRACE_W_T("No photos found in directory %s. Continue...", grayFacesPath);
    }
    else
    {
        labels.clear();
        userGrayFaces.clear();

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
            labels.push_back(0);
            userGrayFaces.push_back(newFace);
        }

        if(!userGrayFaces.empty())
        {
            TRACE_T("Model training started");
            try
            {
                model->train(userGrayFaces, labels);
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
FrokResult FaceModelEigenfaces::GenerateUserModel(std::vector<cv::Mat> &grayFaces)
{
    TRACE_T("started");
    if(grayFaces.empty())
    {
        TRACE_F_T("Invalid parameter: input vector is empty");
        return FROK_RESULT_INVALID_PARAMETER;
    }

    labels.clear();
    userGrayFaces.clear();
    userGrayFaces = grayFaces;

    for(unsigned i = 0; i < grayFaces.size(); i++)
    {
        labels.push_back(0);
    }

    TRACE_T("Model training started");
    TRACE_T("Generating model from %u photos", (unsigned)grayFaces.size());
    try
    {
        model->train(userGrayFaces, labels);
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

FrokResult FaceModelEigenfaces::AddFaceToModel(cv::Mat &grayFace)
{
    TRACE_T("started");
    labels.push_back(0);
    userGrayFaces.push_back(grayFace);
    TRACE_T("Updating model started");
    try
    {
        model->update(userGrayFaces, labels);
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to update model");
        return FROK_RESULT_OPENCV_ERROR;
    }
    TRACE_S_T("Updating model succeed");
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceModelEigenfaces::LoadUserModel(const char *userPath)
{
    TRACE_T("started");
    if(userPath == NULL)
    {
        TRACE_F_T("Invalid parameter: userPath = %p", userPath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    std::string modelPath(userPath);

    modelPath.append(USER_MODEL_FILENAME_EIGENFACES);

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

FrokResult FaceModelEigenfaces::SaveUserModel(const char *userPath)
{
    TRACE_T("started");
    if(userPath == NULL)
    {
        TRACE_F_T("Invalid parameter: userPath = %p", userPath);
        return FROK_RESULT_INVALID_PARAMETER;
    }
    std::string modelPath(userPath);

    modelPath.append(USER_MODEL_FILENAME_EIGENFACES);

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

FrokResult FaceModelEigenfaces::GetPredictedFace(cv::Mat &targetFace, cv::Mat &predictedFace)
{
    TRACE_T("started");
    try
    {
        cv::Mat tempTargetFace;
        targetFace.copyTo(tempTargetFace);
        // Get some required data from the FaceRecognizer model.
        cv::Mat eigenvectors = model->get<cv::Mat>("eigenvectors");
        cv::Mat averageFaceRow = model->get<cv::Mat>("mean");
        // Project the input image onto the eigenspace.
        //cv::Mat projection = cv::subspaceProject(eigenvectors, averageFaceRow, targetFace.reshape(1, 1));
        cv::Mat projection = cv::subspaceProject(eigenvectors, averageFaceRow, tempTargetFace.reshape(1, 1));
        // Generate the reconstructed face back from the eigenspace.
        cv::Mat reconstructionRow = cv::subspaceReconstruct(eigenvectors, averageFaceRow, projection);
        // Make it a rectangular shaped image instead of a single row.
        cv::Mat reconstructionMat = reconstructionRow.reshape(1, targetFace.rows);
        // Convert the floating-point pixels to regular 8-bit uchar.
        predictedFace = cv::Mat(reconstructionMat.size(), CV_8U);
        reconstructionMat.convertTo(predictedFace, CV_8U);
    }
    catch(...)
    {
        TRACE_F_T("Opencv failed to get predicted face");
        return FROK_RESULT_OPENCV_ERROR;
    }

    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}
