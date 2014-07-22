#include "FaceDetector.h"

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
        return FROK_RESULT_CASCADE_ERROR;
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

FrokResult FaceDetector::NormalizeFace(cv::Mat &normalizedFaceImage)
{
    TRACE_T("started...");
    FrokResult res;
    if(FROK_RESULT_SUCCESS != (res = AlignFaceImage(normalizedFaceImage)))
    {
        TRACE_F_T("RotateImage failed on result %x", res);
        return res;
    }

    if(FROK_RESULT_SUCCESS != (res = RemoveDrowbackFrokImage(normalizedFaceImage)))
    {
        TRACE_F_T("RemoveDrowbackFrokImage failed on result %x", res);
        return res;
    }
    TRACE_T("finished");
    return FROK_RESULT_SUCCESS;
}

FrokResult FaceDetector::AlignFaceImage(cv::Mat &image)
{
    // If no value specified for cascades - set default
    for(int i = CASCADE_EYES_BEGIN; i <= CASCADE_EYES_END; i++)
    {
        if(cascades[(EnumCascades)i].properties.minObjectSize.width == -1 ||
                cascades[(EnumCascades)i].properties.minObjectSize.height == -1)
        {
            // TBD Resolve magic numbers problem
            cascades[(EnumCascades)i].properties.minObjectSize = cvSize(image.cols / 6, image.rows / 7);
        }
        if(cascades[(EnumCascades)i].properties.maxObjectSize.width == -1 ||
                cascades[(EnumCascades)i].properties.maxObjectSize.height == -1)
        {
            cascades[(EnumCascades)i].properties.maxObjectSize = cvSize(image.cols / 3, image.rows / 4);
        }
    }

    if(cascades[CASCADE_NOSE_MSC].properties.minObjectSize.width == -1 ||
            cascades[CASCADE_NOSE_MSC].properties.minObjectSize.height == -1)
    {
        cascades[CASCADE_NOSE_MSC].properties.minObjectSize = cvSize(image.cols / 5, image.rows / 6);
    }

    if(cascades[CASCADE_NOSE_MSC].properties.maxObjectSize.width == -1 ||
            cascades[CASCADE_NOSE_MSC].properties.maxObjectSize.height == -1)
    {
        cascades[CASCADE_NOSE_MSC].properties.maxObjectSize = cvSize((int)(image.cols / 3.5), (int)(image.rows / 3.5));
    }

    if(cascades[CASCADE_MOUTH_MSC].properties.minObjectSize.width == -1 ||
            cascades[CASCADE_MOUTH_MSC].properties.minObjectSize.height == -1)
    {
        cascades[CASCADE_MOUTH_MSC].properties.minObjectSize = cvSize(image.cols / 5, image.rows / 6);
    }

    if(cascades[CASCADE_MOUTH_MSC].properties.maxObjectSize.width == -1 ||
            cascades[CASCADE_MOUTH_MSC].properties.maxObjectSize.height == -1)
    {
        cascades[CASCADE_MOUTH_MSC].properties.maxObjectSize = cvSize(image.cols / 2, image.rows / 2);
    }
    // End of setting defaults

    // [TBD] Nikita resized image to image * 5 if width/height < 200. I do not.

    HumanFace humanFace;
    bool leftEyeFound = false;
    bool rightEyeFound = false;
    bool noseFound = false;
    bool mouthFound = false;

    // Try to detect eyes
    std::vector < cv::Rect > eyes;
    TRACE_T("Detecting eyes with CASCADE_EYE...");
    cascades[CASCADE_EYE].cascade.detectMultiScale(image, eyes,
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
    cascades[CASCADE_EYE_WITH_GLASSES].cascade.detectMultiScale(image, eyes,
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
    cascades[CASCADE_EYE_LEFT].cascade.detectMultiScale(image, eyes,
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
        cascades[CASCADE_EYE_LEFT_SPLITTED].cascade.detectMultiScale(image, eyes,
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
    cascades[CASCADE_EYE_RIGHT].cascade.detectMultiScale(image, eyes,
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
        cascades[CASCADE_EYE_RIGHT_SPLITTED].cascade.detectMultiScale(image, eyes,
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
    cascades[CASCADE_NOSE_MSC].cascade.detectMultiScale(image, nose,
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
    cascades[CASCADE_MOUTH_MSC].cascade.detectMultiScale(image, mouth,
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

    if(leftEyeFound) cv::imwrite("/home/zda/faces/LE.jpg", cv::Mat(image, humanFace.leftEye));
    if(rightEyeFound) cv::imwrite("/home/zda/faces/RE.jpg", cv::Mat(image, humanFace.rightEye));
    if(leftEyeFound) cv::imwrite("/home/zda/faces/nose.jpg", cv::Mat(image, humanFace.nose));
    if(leftEyeFound) cv::imwrite("/home/zda/faces/mouth.jpg", cv::Mat(image, humanFace.mouth));


    TRACE_S_T("Detection finished");
    /*

    cscd->detectMultiScale((Mat)dst, objects,scale_factor, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, minSize, maxSize);
    //objects = cvHaarDetectObjects(dst, cscd, strg, scale_factor, 3, 0 | CV_HAAR_DO_CANNY_PRUNING, minSize, maxSize);

    for(vector<Rect>::iterator it = objects.begin(); it != objects.end(); ++it)
    {
        int x = cvRound(((Rect)(*it)).x) / k;
        int y = cvRound(((Rect)(*it)).y) / k;
        int w = cvRound(((Rect)(*it)).width) / k;
        int h = cvRound(((Rect)(*it)).height) / k;

        //CvPoint p1 = cvPoint(x + pointFace.x, y + pointFace.y), p2 = cvPoint(x + w + pointFace.x, y + h + pointFace.y);
        ImageCoordinats pointKeyFace, facePointCoordinates;

        pointKeyFace.p1 = cvPoint(x + pointFace.x, y + pointFace.y);
        pointKeyFace.p2 = cvPoint(x + w + pointFace.x, y + h + pointFace.y);
        facePointCoordinates.p1 = pointFace;
        facePointCoordinates.p2 = cvPoint(pointFace.x + width / k, pointFace.y + height / k);


        writeFacePoints(pointKeyFace, facePointCoordinates, type);
    }

    cvReleaseImage(&dst);
    objects.clear();

    // NORMALIZE

    Ptr<CLAHE> clahe = createCLAHE(2, Size(8, 8));
    clahe->apply(Mat(face_img), Mat(face_img));
    cvNormalize(face_img, face_img, 10, 250, CV_MINMAX);


    // DRAW EVIDENCE

    const CvPoint p1 = pointFace.p1;
    const CvPoint p2 = pointFace.p2;

    int count = 0;
    for (int i = 0; i < 8; i++)
    {
        if (facePoints[i].x >= 0 && facePoints[i].y >= 0)
        {
            count++;
        }
    }

    if (count >= 4){        //[TBD] Why 4?
        return true;
    }
    return false;

    // DEFINE ROTATE

    double rad = 57.295779513;
    CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);

    if ((facePoints[0].x > 0 && facePoints[0].y > 0) && (facePoints[1].x > 0 && facePoints[1].y > 0))
    {
        CvPoint pa = cvPoint((facePoints[0].x + facePoints[4].x) / 2, (facePoints[0].y + facePoints[4].y) / 2);
        CvPoint pb = cvPoint((facePoints[1].x + facePoints[5].x) / 2, (facePoints[1].y + facePoints[5].y) / 2);

        double x = (pb.x - pa.x);
        double y = (pb.y - pa.y);
        CvPoint2D32f center;

        center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);

        double angle = atan(y / x)*rad;

        cv2DRotationMatrix(center, angle, 1, transmat);
        cvWarpAffine(face_img, face_img, transmat);
        cvReleaseMat(&transmat);
        return 0;
    }
    else if ((facePoints[2].x > 0 && facePoints[2].y > 0) && (facePoints[3].x > 0 && facePoints[3].y > 0))
    {
        CvPoint pa = cvPoint((facePoints[2].x + facePoints[6].x) / 2, (facePoints[2].y + facePoints[6].y) / 2);
        CvPoint pb = cvPoint((facePoints[3].x + facePoints[7].x) / 2, (facePoints[3].y + facePoints[7].y) / 2);

        CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);
        double x = (pb.x - pa.x);
        double y = (pb.y - pa.y);
        CvPoint2D32f center;

        center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);
        double angle = 0;
        angle = atan(y / x)*rad;

        if (abs(angle) > 30){
            cvReleaseMat(&transmat);
            return -1;
        }
        if (angle > 0)  angle -= 90;
        else if (angle < 0) angle += 90;
        else angle = 90;

        cv2DRotationMatrix(center, angle, 1, transmat);
        cvWarpAffine(face_img, face_img, transmat);
        cvReleaseMat(&transmat);
        return 0;
    }
    return -1;

    // IMPOSE MASK

    int x, y, width, height, width_roi, height_roi;

    width = face_img->width;
    height = face_img->height;

    x = (int)(width / 5);
    y = (int)(height / 4);
    width_roi = width - x * 2;
    height_roi = height;

    IplImage *img = cvCreateImage(cvSize(width - x * 2, height - y), face_img->depth, face_img->nChannels);

    cvSetImageROI(face_img, cvRect(x, y, width_roi, height_roi));
    cvCopy(face_img, img, NULL);
    cvResetImageROI(face_img);

    return img;

    //MAIN

    for (int j = 0; j < 8; j++)
        facePoints[j] = cvPoint(-1, -1);

    allKeysFaceDetection(points.p1);
    normalizateHistFace();

    if (!drawEvidence(points)){
        return false;
    }
    if (defineRotate() != 0){
        return false;
    }

    face_img = imposeMask(points.p1);
    IplImage *temporary = new IplImage(eigenDetector->MaskFace(face_img));
    face_img

    /*double rad = 57.295779513;
   CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);

    if ((facePoints[0].x > 0 && facePoints[0].y > 0) && (facePoints[1].x > 0 && facePoints[1].y > 0))
    {
        int w = face_img->width;
        int h = face_img->height;

        CvPoint pa = cvPoint((facePoints[0].x + facePoints[4].x) / 2, (facePoints[0].y + facePoints[4].y) / 2);
        CvPoint pb = cvPoint((facePoints[1].x + facePoints[5].x) / 2, (facePoints[1].y + facePoints[5].y) / 2);

        double x = (pb.x - pa.x);
        double y = (pb.y - pa.y);
        CvPoint2D32f center;

        center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);

        double angle = atan(y / x)*rad;

        cv2DRotationMatrix(center, angle, 1, transmat);
        cvWarpAffine(face_img, face_img, transmat);
        cvReleaseMat(&transmat);
        return 0;
    }
    else if ((facePoints[2].x > 0 && facePoints[2].y > 0) && (facePoints[3].x > 0 && facePoints[3].y > 0)){

        int w = face_img->width;
        int h = face_img->height;

        CvPoint pa = cvPoint((facePoints[2].x + facePoints[6].x) / 2, (facePoints[2].y + facePoints[6].y) / 2);
        CvPoint pb = cvPoint((facePoints[3].x + facePoints[7].x) / 2, (facePoints[3].y + facePoints[7].y) / 2);

        CvMat *transmat = cvCreateMat(2, 3, CV_32FC1);
        double x = (pb.x - pa.x);
        double y = (pb.y - pa.y);
        CvPoint2D32f center;

        center = cvPoint2D32f(face_img->width / 2, face_img->height / 2);
        double angle = 0;
        angle = atan(y / x)*rad;

        if (abs(angle) > 30){
            cvReleaseMat(&transmat);
            return -1;
        }
        if (angle > 0)  angle -= 90;
        else if (angle < 0) angle += 90;
        else angle = 90;

        cv2DRotationMatrix(center, angle, 1, transmat);
        cvWarpAffine(face_img, face_img, transmat);
        cvReleaseMat(&transmat);
        return 0;
    }*/
}

FrokResult FaceDetector::RemoveDrowbackFrokImage(cv::Mat &image)
{}
