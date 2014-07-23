#include "FaceDetector.h"

#include <math.h>

#define MODULE_NAME         "FACE_DETECTOR"

FaceDetector::FaceDetector()
{
    StructCascade cascade;
// Set Face cascade defaults
    cascade.properties.maxObjectSize = cvSize(-1, -1);
    cascade.properties.minObjectSize = cvSize(40, 50);
    cascade.properties.minNeighbors = 3;
    cascade.properties.scaleFactor = 1.1;
    cascades[CASCADE_FACE] = cascade;
    cascades[CASCADE_FACE].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml");

// Set Eyes cascade defaults
    cascade.properties.scaleFactor = 1.01;
    cascade.properties.minNeighbors = 3;
    cascade.properties.maxObjectSize = cvSize(-1, -1);
    cascade.properties.minObjectSize = cvSize(-1, -1);

    for(unsigned i = CASCADE_EYES_BEGIN; i <= CASCADE_EYES_END; i++)
    {
        cascades[(EnumCascades)i] = cascade;
    }
    cascades[CASCADE_EYE].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye.xml");
    cascades[CASCADE_EYE_WITH_GLASSES].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml");
    cascades[CASCADE_EYE_RIGHT].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_righteye.xml");
    cascades[CASCADE_EYE_LEFT].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_lefteye.xml");
    cascades[CASCADE_EYE_RIGHT_SPLITTED].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_righteye_2splits.xml");
    cascades[CASCADE_EYE_LEFT_SPLITTED].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_lefteye_2splits.xml");

// Set Nose cascade defaults
    cascade.properties.scaleFactor = 1.2;
    cascade.properties.minNeighbors = 3;
    cascade.properties.maxObjectSize = cvSize(-1, -1);
    cascade.properties.minObjectSize = cvSize(-1, -1);
    cascades[CASCADE_NOSE_MSC] = cascade;
    cascades[CASCADE_NOSE_MSC].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_nose.xml");

// Set Mouth cascade defaults
    cascade.properties.scaleFactor = 1.4;
    cascade.properties.minNeighbors = 3;
    cascade.properties.maxObjectSize = cvSize(-1, -1);
    cascade.properties.minObjectSize = cvSize(-1, -1);
    cascades[CASCADE_MOUTH_MSC] = cascade;
    cascades[CASCADE_MOUTH_MSC].cascade.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml");

    normalizerClahe = cv::createCLAHE(2, cv::Size(8, 8));

    aligningScaleFactor = 1;

    TRACE("new FaceDetector");
}

FaceDetector::~FaceDetector()
{
    TRACE("~FaceDetector");
}

FrokResult FaceDetector::SetCascadeParameters(EnumCascades cascade, CascadeProperties params)
{
    if(cascades.find(cascade) == cascades.end())
    {
        TRACE_F("Invalid cascade name");
        return FROK_RESULT_INVALID_PARAMETER;
    }

    cascades[cascade].properties = params;
}

FrokResult FaceDetector::SetTargetImage(const char *imagePath)
{
    TRACE_T("started...");
    targetImageKappa = cv::imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
    if(!targetImageKappa.data)
    {
        TRACE_F_T("Failed to open image %s", imagePath);
        return FROK_RESULT_INVALID_PARAMETER;
    }

    cascades[CASCADE_FACE].properties.maxObjectSize = cv::Size(targetImageKappa.cols, targetImageKappa.rows);
    TRACE_T("finished");
}

FrokResult FaceDetector::GetFacesFromPhoto(std::vector< cv::Rect > &faces)
{
    TRACE_T("started...");
    try
    {
        cascades[CASCADE_FACE].cascade.detectMultiScale(targetImageKappa, faces, cascades[CASCADE_FACE].properties.scaleFactor,
                                       cascades[CASCADE_FACE].properties.minNeighbors, 0,
                                       cascades[CASCADE_FACE].properties.minObjectSize,
                                       cascades[CASCADE_FACE].properties.maxObjectSize);
    }
    catch(...)
    {
        TRACE_F_T("detectMultiScale Failed");
        return FROK_RESULT_NOT_A_FACE;
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}
FrokResult FaceDetector::GetFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages)
{
    TRACE_T("started...");

    if(coords.empty())
    {
        return FROK_RESULT_INVALID_PARAMETER;
    }
    for(std::vector<cv::Rect>::iterator it = coords.begin(); it != coords.end(); ++it)
    {
        cv::Rect faceCoord = (cv::Rect)*it;
        cv::Mat faceImage(targetImageKappa, faceCoord);
        faceImages.push_back(faceImage);
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceDetector::NormalizeFace(cv::Rect &normalizedFaceImage)
{
    TRACE_T("started...");
    FrokResult res;

    //normalizerClahe->apply(normalizedFaceImage, normalizedFaceImage);
    //cv::normalize()  cvNormalize(image, image, 10, 250, CV_MINMAX);


    if(FROK_RESULT_SUCCESS != (res = AlignFaceImage(normalizedFaceImage, targetImageKappa)))
    {
        TRACE_F_T("RotateImage failed on result %x", res);
        return res;
    }

    /*if(FROK_RESULT_SUCCESS != (res = RemoveDrowbackFrokImage(normalizedFaceImage)))
    {
        TRACE_F_T("RemoveDrowbackFrokImage failed on result %x", res);
        return res;
    }*/
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceDetector::AlignFaceImage(cv::Rect faceCoords, cv::Mat &alignedFaceImage)
{
    TRACE_T("started...");
    FrokResult result;
    // If no value specified for cascades - set default
    for(int i = CASCADE_EYES_BEGIN; i <= CASCADE_EYES_END; i++)
    {
        if(cascades[(EnumCascades)i].properties.minObjectSize.width == -1 ||
                cascades[(EnumCascades)i].properties.minObjectSize.height == -1)
        {
            // TBD Resolve magic numbers problem
            cascades[(EnumCascades)i].properties.minObjectSize = cvSize(faceCoords.width / 6, faceCoords.height / 7);
        }
        if(cascades[(EnumCascades)i].properties.maxObjectSize.width == -1 ||
                cascades[(EnumCascades)i].properties.maxObjectSize.height == -1)
        {
            cascades[(EnumCascades)i].properties.maxObjectSize = cvSize(faceCoords.width / 3, faceCoords.height / 4);
        }
    }

    if(cascades[CASCADE_NOSE_MSC].properties.minObjectSize.width == -1 ||
            cascades[CASCADE_NOSE_MSC].properties.minObjectSize.height == -1)
    {
        cascades[CASCADE_NOSE_MSC].properties.minObjectSize = cvSize(faceCoords.width / 5, faceCoords.height / 6);
    }

    if(cascades[CASCADE_NOSE_MSC].properties.maxObjectSize.width == -1 ||
            cascades[CASCADE_NOSE_MSC].properties.maxObjectSize.height == -1)
    {
        cascades[CASCADE_NOSE_MSC].properties.maxObjectSize = cvSize((int)(faceCoords.width / 3.5), (int)(faceCoords.height / 3.5));
    }

    if(cascades[CASCADE_MOUTH_MSC].properties.minObjectSize.width == -1 ||
            cascades[CASCADE_MOUTH_MSC].properties.minObjectSize.height == -1)
    {
        cascades[CASCADE_MOUTH_MSC].properties.minObjectSize = cvSize(faceCoords.width / 5, faceCoords.height / 6);
    }

    if(cascades[CASCADE_MOUTH_MSC].properties.maxObjectSize.width == -1 ||
            cascades[CASCADE_MOUTH_MSC].properties.maxObjectSize.height == -1)
    {
        cascades[CASCADE_MOUTH_MSC].properties.maxObjectSize = cvSize(faceCoords.width / 2, faceCoords.height / 2);
    }
    // End of setting defaults

    // [TBD] Nikita resized image to image * 5 if width/height < 200. I do not.

    cv::Mat imageFace(targetImageKappa, faceCoords);
    TRACE_T("Detection started");

    HumanFace humanFace;
    bool leftEyeFound = false;
    bool rightEyeFound = false;
    bool noseFound = false;
    bool mouthFound = false;

    // Try to detect eyes
    std::vector < cv::Rect > eyes;
    TRACE_T("Detecting eyes with CASCADE_EYE...");
    cascades[CASCADE_EYE].cascade.detectMultiScale(imageFace, eyes,
                                                    cascades[CASCADE_EYE].properties.scaleFactor,
                                                    cascades[CASCADE_EYE].properties.minNeighbors,
                                                    0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                    cascades[CASCADE_EYE].properties.minObjectSize,
                                                    cascades[CASCADE_EYE].properties.maxObjectSize);

    if(eyes.size() == 2)
    {
        TRACE_S_T("2 eyes were successfully detected with CASCADE_EYE");
        // [TBD] Left eye is the left one or the one that is to the left on the photo?
        TRACE_W("Left eye is the left one or the one that is to the left on the photo?");
        if(eyes[0].x > eyes[1].x)
        {
            humanFace.leftEye = eyes[0];
            humanFace.rightEye = eyes[1];
        }
        else
        {
            humanFace.rightEye = eyes[0];
            humanFace.leftEye = eyes[1];
        }
        leftEyeFound = true;
        rightEyeFound = true;
        goto detect_nose;
    }
    TRACE_T("Failed to find 2 eyes with CASCADE_EYE");
    TRACE_T("Now trying to detect eyes in glasses");

    eyes.clear();
    TRACE_T("Detecting eyes with CASCADE_EYE_WITH_GLASSES...");
    cascades[CASCADE_EYE_WITH_GLASSES].cascade.detectMultiScale(imageFace, eyes,
                                                    cascades[CASCADE_EYE_WITH_GLASSES].properties.scaleFactor,
                                                    cascades[CASCADE_EYE_WITH_GLASSES].properties.minNeighbors,
                                                    0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                    cascades[CASCADE_EYE_WITH_GLASSES].properties.minObjectSize,
                                                    cascades[CASCADE_EYE_WITH_GLASSES].properties.maxObjectSize);

    if(eyes.size() == 2)
    {
        TRACE_S_T("2 eyes were successfully detected with CASCADE_EYE_WITH_GLASSES");
        // [TBD] Left eye is the left one or the one that is to the left on the photo?
        TRACE_W("Left eye is the left one or the one that is to the left on the photo?");
        if(eyes[0].x > eyes[1].x)
        {
            humanFace.leftEye = eyes[0];
            humanFace.rightEye = eyes[1];
        }
        else
        {
            humanFace.rightEye = eyes[0];
            humanFace.leftEye = eyes[1];
        }
        leftEyeFound = true;
        rightEyeFound = true;
        goto detect_nose;
    }
    TRACE_T("Failed to find 2 eyes with CASCADE_EYE_WITH_GLASSES");
    TRACE_T("Now trying to detect left eye with CASCADE_EYE_LEFT and CASCADE_EYE_LEFT_SPLITTED");

    eyes.clear();
    TRACE_T("Detecting left eye with CASCADE_EYE_LEFT...");
    cascades[CASCADE_EYE_LEFT].cascade.detectMultiScale(imageFace, eyes,
                                                    cascades[CASCADE_EYE_LEFT].properties.scaleFactor,
                                                    cascades[CASCADE_EYE_LEFT].properties.minNeighbors,
                                                    0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                    cascades[CASCADE_EYE_LEFT].properties.minObjectSize,
                                                    cascades[CASCADE_EYE_LEFT].properties.maxObjectSize);

    if(eyes.size() == 1)
    {
        TRACE_S_T("Left eye was successfully detected with CASCADE_EYE_LEFT");
        // [TBD] Left eye is the left one or the one that is to the left on the photo?
        TRACE_W("Left eye is the left one or the one that is to the left on the photo?");
        humanFace.leftEye = eyes[0];
        leftEyeFound = true;
    }
    else
    {
        eyes.clear();
        TRACE_T("Detecting left eye with CASCADE_EYE_LEFT_SPLITTED...");
        cascades[CASCADE_EYE_LEFT_SPLITTED].cascade.detectMultiScale(imageFace, eyes,
                                                        cascades[CASCADE_EYE_LEFT_SPLITTED].properties.scaleFactor,
                                                        cascades[CASCADE_EYE_LEFT_SPLITTED].properties.minNeighbors,
                                                        0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                        cascades[CASCADE_EYE_LEFT_SPLITTED].properties.minObjectSize,
                                                        cascades[CASCADE_EYE_LEFT_SPLITTED].properties.maxObjectSize);
        if(eyes.size() == 1)
        {
            TRACE_S_T("Left eye was successfully detected with CASCADE_EYE_LEFT_SPLITTED");
            // [TBD] Left eye is the left one or the one that is to the left on the photo?
            TRACE_W("Left eye is the left one or the one that is to the left on the photo?");
            humanFace.leftEye = eyes[0];
            leftEyeFound = true;
        }
    }


    eyes.clear();
    TRACE_T("Detecting right eye with CASCADE_EYE_RIGHT...");
    cascades[CASCADE_EYE_RIGHT].cascade.detectMultiScale(imageFace, eyes,
                                                    cascades[CASCADE_EYE_RIGHT].properties.scaleFactor,
                                                    cascades[CASCADE_EYE_RIGHT].properties.minNeighbors,
                                                    0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                    cascades[CASCADE_EYE_RIGHT].properties.minObjectSize,
                                                    cascades[CASCADE_EYE_RIGHT].properties.maxObjectSize);

    if(eyes.size() == 1)
    {
        TRACE_S_T("Right eye was successfully detected with CASCADE_EYE_RIGHT");
        // [TBD] Left eye is the left one or the one that is to the left on the photo?
        TRACE_W("Left eye is the left one or the one that is to the left on the photo?");
        humanFace.rightEye = eyes[0];
        rightEyeFound = true;
    }
    else
    {
        eyes.clear();
        TRACE_T("Detecting right eye with CASCADE_EYE_RIGHT_SPLITTED...");
        cascades[CASCADE_EYE_RIGHT_SPLITTED].cascade.detectMultiScale(imageFace, eyes,
                                                        cascades[CASCADE_EYE_RIGHT_SPLITTED].properties.scaleFactor,
                                                        cascades[CASCADE_EYE_RIGHT_SPLITTED].properties.minNeighbors,
                                                        0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                        cascades[CASCADE_EYE_RIGHT_SPLITTED].properties.minObjectSize,
                                                        cascades[CASCADE_EYE_RIGHT_SPLITTED].properties.maxObjectSize);
        if(eyes.size() == 1)
        {
            TRACE_S_T("Right eye was successfully detected with CASCADE_EYE_RIGHT_SPLITTED");
            // [TBD] Left eye is the left one or the one that is to the left on the photo?
            TRACE_W("Left eye is the left one or the one that is to the left on the photo?");
            humanFace.rightEye = eyes[0];
            rightEyeFound = true;
        }
    }

    if(rightEyeFound == false && leftEyeFound == false)
    {
        TRACE_F_T("Eyes detection failed");
    }
    else if(rightEyeFound == false || leftEyeFound == false)
    {
        TRACE_F_T("Only one eye detected");
    }
    else
    {
        TRACE_S_T("Eyes detection succeed");
    }
detect_nose:
    std::vector < cv::Rect > nose;
    TRACE_T("Detecting eyes with CASCADE_NOSE_MSC...");
    cascades[CASCADE_NOSE_MSC].cascade.detectMultiScale(imageFace, nose,
                                                    cascades[CASCADE_NOSE_MSC].properties.scaleFactor,
                                                    cascades[CASCADE_NOSE_MSC].properties.minNeighbors,
                                                    0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                    cascades[CASCADE_NOSE_MSC].properties.minObjectSize,
                                                    cascades[CASCADE_NOSE_MSC].properties.maxObjectSize);

    if(nose.size() == 1)
    {
        TRACE_S_T("Nose was successfully detected with CASCADE_NOSE_MSC");
        humanFace.nose = nose[0];
        noseFound = true;
        goto detect_mouth;
    }
    TRACE_T("Failed to find nose with CASCADE_NOSE_MSC");
    TRACE_F_T("Nose detection failed");
detect_mouth:
    std::vector < cv::Rect > mouth;
    TRACE_T("Detecting eyes with CASCADE_NOSE_MSC...");
    cascades[CASCADE_MOUTH_MSC].cascade.detectMultiScale(imageFace, mouth,
                                                    cascades[CASCADE_MOUTH_MSC].properties.scaleFactor,
                                                    cascades[CASCADE_MOUTH_MSC].properties.minNeighbors,
                                                    0 | CV_HAAR_DO_CANNY_PRUNING,  // This is marked as legacy
                                                    cascades[CASCADE_MOUTH_MSC].properties.minObjectSize,
                                                    cascades[CASCADE_MOUTH_MSC].properties.maxObjectSize);

    if(mouth.size() == 1)
    {
        TRACE_S_T("Mouth was successfully detected with CASCADE_MOUTH_MSC");
        humanFace.mouth = mouth[0];
        mouthFound = true;
        goto detect_finish;
    }
    TRACE_T("Failed to find mouth with CASCADE_MOUTH_MSC");
    TRACE_F_T("Mouth detection failed");

detect_finish:
    TRACE_S_T("Detection finished");

    double radiansToDegrees = 360 / (2 * M_PI);
    cv::Mat transMat(2, 3, CV_32FC1);

    TRACE_T("Aligning started");
    if(leftEyeFound && rightEyeFound)
    {
        TRACE_T("Aligning by left and right eyes");

        CvPoint centerLeftEye = cvPoint(humanFace.leftEye.x + humanFace.leftEye.width / 2, humanFace.leftEye.y + humanFace.leftEye.height / 2);
        CvPoint centerRightEye = cvPoint(humanFace.rightEye.x + humanFace.rightEye.width / 2, humanFace.rightEye.y + humanFace.rightEye.height / 2);
        cv::Point2f centerImage(faceCoords.x + faceCoords.width / 2, faceCoords.y + faceCoords.height / 2);
        double aligningAngle = atan(((double)centerRightEye.y - centerLeftEye.y) / ((double)centerRightEye.x - centerLeftEye.x)) * radiansToDegrees;

        try
        {
            cv::Mat temporaryImageForAlign(targetImageKappa);

            transMat = cv::getRotationMatrix2D(centerImage, aligningAngle, aligningScaleFactor);
            cv::warpAffine(temporaryImageForAlign, temporaryImageForAlign, transMat, cv::Size(temporaryImageForAlign.cols, temporaryImageForAlign.rows));
            alignedFaceImage = cv::Mat(temporaryImageForAlign, faceCoords);
        }
        catch(...)
        {
            TRACE_F_T("Opencv image rotation failed");
            result = FROK_RESULT_OPENCV_ERROR;
            goto align_finish;
        }
    }
    else if(noseFound && mouthFound)
    {
        TRACE_T("aligning by mouth and nose");

        CvPoint centerNose = cvPoint(humanFace.nose.x + humanFace.nose.width / 2, humanFace.nose.y + humanFace.nose.height / 2);
        CvPoint centerMouth = cvPoint(humanFace.mouth.x + humanFace.mouth.width / 2, humanFace.mouth.y + humanFace.mouth.height / 2);
        cv::Point2f centerImage(faceCoords.x + faceCoords.width / 2, faceCoords.y + faceCoords.height / 2);
        double aligningAngle = atan(((double)centerMouth.y - centerNose.y) / ((double)centerMouth.x - centerNose.x)) * radiansToDegrees;

        // [TBD] Why 30?
        if (aligningAngle > 30.0)
        {
            result = FROK_RESULT_NOT_A_FACE;
            goto align_finish;
        }
        if (aligningAngle > 0)          aligningAngle -= 90;
        else if (aligningAngle <= 0)     aligningAngle += 90;

        try
        {
            cv::Mat temporaryImageForAlign(targetImageKappa);

            transMat = cv::getRotationMatrix2D(centerImage, aligningAngle, aligningScaleFactor);
            cv::warpAffine(temporaryImageForAlign, temporaryImageForAlign, transMat, cv::Size(temporaryImageForAlign.cols, temporaryImageForAlign.rows));
            alignedFaceImage = cv::Mat(temporaryImageForAlign, faceCoords);
        }
        catch(...)
        {
            TRACE_F_T("Opencv image rotation failed");
            result = FROK_RESULT_OPENCV_ERROR;
            goto align_finish;
        }
    }
    else
    {
        TRACE_F_T("Not enaugh information for aligning");
        result = FROK_RESULT_NOT_A_FACE;
        goto align_finish;
    }

    TRACE_S_T("Aligning succeed");

    result = FROK_RESULT_SUCCESS;
align_finish:
    TRACE_T("finished");
    return result;
}

FrokResult FaceDetector::RemoveDrowbackFrokImage(cv::Mat &image)
{}
