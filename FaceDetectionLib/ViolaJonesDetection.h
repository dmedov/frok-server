#ifndef VIOLAJONESDETECTION_H
#define VIOLAJONESDETECTION_H

#pragma pack(push, 1)

struct FaceCascades{
    CascadeClassifier *face;
    CascadeClassifier *eyes;
    CascadeClassifier *righteye;
    CascadeClassifier *lefteye;
    CascadeClassifier *righteye2;
    CascadeClassifier *lefteye2;
    CascadeClassifier *eye;
    CascadeClassifier *nose;
    CascadeClassifier *mouth;
    FaceCascades()
    {
        face = new CascadeClassifier;
        eyes = new CascadeClassifier;
        righteye = new CascadeClassifier;
        lefteye = new CascadeClassifier;
        righteye2 = new CascadeClassifier;
        lefteye2 = new CascadeClassifier;
        eye = new CascadeClassifier;
        mouth = new CascadeClassifier;
        nose = new CascadeClassifier;
    }
    ~FaceCascades()
    {
        delete face;
        delete eyes;
        delete righteye;
        delete lefteye;
        delete righteye2;
        delete lefteye2;
        delete eye;
        delete mouth;
        delete nose;
        printf("~FaceCascades\n");
    }
};

class ViolaJonesDetection
{
private:
    IplImage *image;
    IplImage *imageResults;
    IplImage *face_img;
    IplImage *gray_img;
    CvPoint facePoints[8];
    CvMemStorage* strg;
    FaceCascades *faceCascades;
public:
    ViolaJonesDetection(FaceCascades *cascades);
    ~ViolaJonesDetection();
    bool allFacesDetection(IplImage *inputImage, SOCKET outSock);
    bool cutFaceToBase(IplImage* bigImage, const char *destPath, int x, int y, int w, int h);
    void keysFaceDetectFromForeignImage(CvHaarClassifierCascade* cscd, IplImage *face, int type, CvPoint facePoints[]);
    bool faceDetect(IplImage *inputImage, const map < string, Ptr<FaceRecognizer> > &models, SOCKET outSock = INVALID_SOCKET);
    static void cutFaceThread(void *params);
    bool cutTheFace(IplImage *inputImage, const char* destPath, int faceNumber);
private:
    bool drawEvidence(const ImageCoordinats &pointFace);
    void writeFacePoints(const ImageCoordinats &pointKeyFace, const ImageCoordinats &pointFace, int type);
    void keysFaceDetect(CascadeClassifier* cscd, CvPoint pointFace, int type);
    void allKeysFaceDetection(CvPoint point);
    int defineRotate();
    IplImage* imposeMask(CvPoint p);
    void createJson(const DataJson &dataJson, SOCKET sock);        // [TBD] change it to smth like show on photo or send response etc
    void normalizateHistFace();
};

struct cutFaceThreadParams
{
    IplImage *inputImage;
    char* destPath;
    ViolaJonesDetection *pThis;

    cutFaceThreadParams(IplImage *inputImage, const char* destPath, FaceCascades *pCascade)
    {
        this->inputImage = new IplImage;
        this->inputImage = inputImage;
        this->destPath = new char[strlen(destPath)];
        strcpy(this->destPath, destPath);
        pThis = new ViolaJonesDetection(pCascade);
    }
    ~cutFaceThreadParams()
    {
        cvReleaseImage(&inputImage);
        delete pThis;
    }
};
#pragma pack(pop)
#endif //VIOLAJONESDETECTION_H
