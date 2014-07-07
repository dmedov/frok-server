#include "stdafx.h"
#include "LibInclude.h"
#include "common.h"
#include "EigenDetector.h"
#include "DescriptorDetection.h"
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "opencv2/contrib/contrib.hpp"

void EigenDetector::loadBaseFace(const char* facesPath, vector<Mat> * images, vector<int>* labels, int id)
{
    IplImage *base_face = 0;
    vector<string> files = vector<string>();
    getFilesFromDir(facesPath, files);

    for (unsigned int i = 0; i < files.size(); i++)
    {
        IplImage *dist = NULL;
        try
        {
            dist = cvLoadImage(files[i].c_str(), CV_LOAD_IMAGE_GRAYSCALE);
        }
        catch (...)
        {
            FilePrintMessage(NULL, _FAIL("failed to load image %s"), files[i].c_str());
            continue;
        }
        IplImage *resize = cvCreateImage(cvSize(158, 190), dist->depth, dist->nChannels);
        cvResize(dist, resize, 1);

        images->push_back(Mat(resize, true));
        labels->push_back(id);
    }

    cvReleaseImage(&base_face);
}

bool EigenDetector::train(const char* idPath){

    string facesPath = ((string)idPath).append("/faces/");

    vector<Mat> images;
    vector<int> labels;
    Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
    try
    {
        loadBaseFace(facesPath.c_str(), &images, &labels, 0);
    }
    catch (...)
    {
        FilePrintMessage(NULL, _FAIL("Failed to load images for learning (path = %s)."), facesPath.c_str());
        return false;
    }

    try
    {
        model->train(images, labels);
    }
    catch (...)
    {
        FilePrintMessage(NULL, _FAIL("Failed to train model."));
        return false;
    }

    try
    {
        model->save(((string)idPath).append("/eigenface.yml").c_str());
    }
    catch (...)
    {
        FilePrintMessage(NULL, _FAIL("Failed to save %s"), (((string)idPath).append("/eigenface.yml")).c_str());
        return false;
    }

    FilePrintMessage(NULL, _SUCC("Learning completed. See resulting file eigenface.yml in %s folder"), idPath);
    return true;
}

Mat EigenDetector::MaskFace(IplImage *img) {
    Mat _img = Mat(img, true);
    int x = _img.cols;
    int y = _img.rows;
    Mat mask = Mat(_img.size(), CV_8UC1, Scalar(255));
    Point faceCenter = Point(cvRound(x * 0.5),
        cvRound(y * 0.25));
    Size size = Size(cvRound(x * 0.55), cvRound(y * 0.8));
    ellipse(mask, faceCenter, size, 0, 0, 360, Scalar(0),
        CV_FILLED);
    // Apply the elliptical mask on the face, to remove corners.
    // Sets corners to gray, without touching the inner face.

    _img.setTo(Scalar(128), mask);

    return _img;
}

double EigenDetector::getSimilarity(const Mat *image_mat, const Mat *reconstructedFace)
{
    IplImage *blr_img = cvCloneImage((IplImage*)image_mat);
    IplImage *blr_rec = cvCloneImage((IplImage*)reconstructedFace);
    cvErode(blr_img, blr_img, 0, 0);
    cvErode(blr_rec, blr_rec, 0, 0);

    Mat dif = abs(Mat(blr_img) - Mat(blr_rec));

    int koef = 0;
    double err = 0;
    for (int y(0); y < dif.rows; ++y){
        for (int x(0); x < dif.cols; ++x){
            int d = dif.at<unsigned char>(y, x);
            if (d >= 40)
                koef += d;
        }
    }

    err = (double)koef / ((double)dif.cols * (double)dif.rows * 40);

    double prob = (1 - err);

    if (prob > 1) prob = 0.99;
    if (prob < 0) prob = 0.01;


    return prob;

}

// Compare two images by getting the L2 error (square-root of sum of squared error).
double EigenDetector::getSimilarity3(const Mat *projected_mat, const Mat *face_mat)
{
    if ((projected_mat->rows > 0) && (projected_mat->rows == projected_mat->rows ) &&
        (projected_mat->cols > 0) && (projected_mat->cols == projected_mat->cols)) {
        // Calculate the L2 relative error between the 2 images.
        double errorL2 = norm(*projected_mat, *face_mat, CV_L2);
        // Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
        return errorL2 / (double)(projected_mat->rows * projected_mat->cols);;
    }
    else {
        //cout << "WARNING: Images have a different size in 'getSimilarity()'." << endl;
        return 100000000.0;  // Return a bad value            [TBD] this is not invalid value. Value must be really invalid
    }
}


double EigenDetector::getSimilarity2(const Mat *projected_mat, const Mat *face_mat) {

    CvSize imagesSize = cvSize(projected_mat->cols, projected_mat->rows);
    IplImage *projectedStorage = cvCreateImage(imagesSize, IPL_DEPTH_32F, 1);
    IplImage *faceSorage = cvCreateImage(imagesSize, IPL_DEPTH_32F, 1);

    cvCornerMinEigenVal((IplImage*)projected_mat, projectedStorage, 20, 7);
    cvCornerMinEigenVal((IplImage*)face_mat, faceSorage, 20, 7);

    Mat *dif_mat = new Mat(abs(Mat(projectedStorage) - Mat(faceSorage)));
    //imshow("ri", projected_mat);
    //imshow("fi", face_mat);
    //imshow("r", Mat(projectedStorage));
    //imshow("f", Mat(faceSorage));
    //imshow("d", dif_mat);

    //cvWaitKey(0);

    //IplImage *dif_img = &(IplImage)dif_mat;

    double err = 0;
    for (int y = 0; y < dif_mat->rows; ++y){
        for (int x = 0; x < dif_mat->cols; ++x){
            err += cvGet2D((IplImage*)dif_mat, y, x).val[0];
        }
    }


    err /= ((double)dif_mat->rows * (double)dif_mat->cols);

    err *= 2.5;
    double prob = (1 - err);
    if (prob > 1) prob = 0.99;
    if (prob < 0) prob = 0.01;
    //prob -= 0.65;
    //prob *= 3;
    cvReleaseImage(&projectedStorage);
    cvReleaseImage(&faceSorage);
    delete dif_mat;
    return prob;
}

double testMatch(IplImage* image, IplImage* rec){
    assert(image != 0);
    assert(rec != 0);

    IplImage* binI = cvCreateImage(cvGetSize(image), 8, 1);
    IplImage* binT = cvCreateImage(cvGetSize(rec), 8, 1);

    cvCanny(image, binI, 10, 200, 3);
    cvCanny(rec, binT, 10, 300, 3);

    Mat image_mat = Mat(binI, true);
    Mat dif_mat = Mat(binT, true);


    CvMemStorage* storage1 = cvCreateMemStorage(0);
    CvMemStorage* storage2 = cvCreateMemStorage(0);
    CvSeq* contoursI = 0, *contoursT = 0;

    //int contoursCont = cvFindContours(binI, storage1, &contoursI, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

    //int contoursCont2 = cvFindContours(binT, storage2, &contoursT, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));


    CvSeq* seqT = 0;
    double perimT = 0;

    if (contoursT != 0){
        for (CvSeq* seq0 = contoursT; seq0 != 0; seq0 = seq0->h_next){
            double perim = cvContourPerimeter(seq0);
            if (perim > perimT){
                perimT = perim;
                seqT = seq0;
            }
        }
    }

    double matchM = 0;
    //int counter = 0;
    if (contoursI != 0 && contoursT != 0){
        for (CvSeq* seq0 = contoursI; seq0 != 0; seq0 = seq0->h_next){
            double match0 = cvMatchShapes(seq0, seqT, CV_CONTOURS_MATCH_I3);
            if (match0 > matchM){
                matchM = match0;
            }
        }
    }

    cvShowImage("binI", binI);
    cvShowImage("binT", binT);


    cvReleaseImage(&binI);
    cvReleaseImage(&binT);
    FilePrintMessage(NULL, _N("match = %lf"), matchM);
    /*double probability = ((double)koef_image / (double)koef_dif) / 1.7;
    if (probability >= 1) probability = 0.99;*/

    return 0;
}

__int64_t calcImageHash(IplImage* src)
{
    if (!src){
        return 0;
    }

    IplImage *res = 0, *bin = 0;

    res = cvCreateImage(cvSize(8, 8), src->depth, src->nChannels);
    bin = cvCreateImage(cvSize(8, 8), IPL_DEPTH_8U, 1);

    cvResize(src, res);

    CvScalar average = cvAvg(res);
    cvThreshold(res, bin, average.val[0], 255, CV_THRESH_BINARY);

    __int64_t hash = 0;

    int i = 0;
    for (int y = 0; y < bin->height; y++) {
        uchar* ptr = (uchar*)(bin->imageData + y * bin->widthStep);
        for (int x = 0; x < bin->width; x++) {
            if (ptr[x]){
                hash |= (__int64_t)1 << i;
            }
            i++;
        }
    }

    cvReleaseImage(&res);
    cvReleaseImage(&bin);

    return hash;
}

__int64_t calcHammingDistance(__int64_t x, __int64_t y)
{
    __int64_t dist = 0, val = x ^ y;

    // Count the number of set bits
    while (val)
    {
        ++dist;
        val &= val - 1;
    }

    return dist;
}


void EigenDetector::recognize(const map < string, Ptr<FaceRecognizer> > &models, DataJson *psDataJson, IplImage* image)
{
    if (psDataJson == NULL)
    {
        FilePrintMessage(NULL, _FAIL("NULL psDataJson received"));
        return;
    }
    double oldProb = 0;        // probability
    string result_name = "-1";


    for (map < string, Ptr<FaceRecognizer> >::const_iterator it = models.begin(); it != models.end(); ++it)
    {
        Ptr<FaceRecognizer> model = (*it).second;

        double prob = 0;

        Mat *image_mat = new Mat(image, true);
        // Get some required data from the FaceRecognizer model.
        Mat eigenvectors = model->get<Mat>("eigenvectors");
        Mat averageFaceRow = model->get<Mat>("mean");
        // Project the input image onto the eigenspace.
        Mat projection = subspaceProject(eigenvectors, averageFaceRow, image_mat->reshape(1, 1));
        // Generate the reconstructed face back from the eigenspace.
        Mat reconstructionRow = subspaceReconstruct(eigenvectors, averageFaceRow, projection);
        // Make it a rectangular shaped image instead of a single row.
        Mat reconstructionMat = reconstructionRow.reshape(1, image->height);
        // Convert the floating-point pixels to regular 8-bit uchar.
        Mat *reconstructedFace = new Mat(reconstructionMat.size(), CV_8U);
        reconstructionMat.convertTo(*reconstructedFace, CV_8U, 1, 0);//-> to introduce to function



        __int64_t hashO = calcImageHash((IplImage*)reconstructedFace);
        __int64_t hashI = calcImageHash((IplImage*)image_mat);
        __int64_t dist = calcHammingDistance(hashO, hashI);//-> to introduce to function


        if (dist <= 18)
        {
            double prob1 = getSimilarity(reconstructedFace, image_mat);
            double prob2 = getSimilarity2(reconstructedFace, image_mat);
            double prob3 = 1 - getSimilarity3(reconstructedFace, image_mat);

            double prob_res1 = pow(prob1*prob2*prob3, 1. / 3);

            double prob_res2 = max(prob3, prob2); //prob_res2 = max(prob_res2, prob1);
            double prob_res3 = min(prob3, prob2); //prob_res3 = min(prob_res3, prob1);

            prob = abs(prob_res1 - abs(prob_res2 - prob_res3))/1.5;


            FilePrintMessage(NULL, _RES("id = %s probability \t= \t%lf \t(%lf | %lf | %lf)"), (*it).first.c_str(), prob, prob1,prob2, prob3);
            //cout << (*it).first << " " << prob1 << "\t" << prob2 << "\t" << prob << endl;

            if (prob > oldProb)
            {
                oldProb = prob;
                result_name = (*it).first;
            }
        }
        delete reconstructedFace;
        delete image_mat;
    }

    char *pcResultName = new char[result_name.length()];
    strcpy(pcResultName, result_name.c_str());

    psDataJson->ids.push_back(pcResultName);
    psDataJson->probs.push_back(oldProb * 100);

}
