#ifndef FrokFaceDetector_H
#define FrokFaceDetector_H
// FaceServer defaults
#define MAX_SOCKET_BUFF_SIZE            (163840)

#ifdef __cplusplus

// include dependencies
#include "FaceDetectorAbstarct.h"

// [TBD] somehow replace paths to haarcascades with path defines
typedef enum
{
    // face cascade
    CASCADE_FACE                = 0x00,

    // EYES cascades
    CASCADE_EYES_BEGIN          = 0x01,

    CASCADE_EYE                 = 0x01,
    CASCADE_EYE_WITH_GLASSES    = 0x02,
    CASCADE_EYE_LEFT            = 0x03,
    CASCADE_EYE_RIGHT           = 0x04,
    CASCADE_EYE_LEFT_SPLITTED   = 0x05,
    CASCADE_EYE_RIGHT_SPLITTED  = 0x06,

    CASCADE_EYES_END            = 0x06,

    // Nose cascade
    CASCADE_NOSE_MSC            = 0x07,
    // Mouth cascade
    CASCADE_MOUTH_MSC           = 0x08
} EnumCascades;

#pragma pack(push, 1)
typedef struct
{
    // Minimum possible object size. Objects smaller than that are ignored.
    cv::Size minObjectSize;
    // Maximum possible object size. Objects smaller than that are ignored.
    cv::Size maxObjectSize;
    // Parameter specifying how much the image size is reduced at each image scale
    double scaleFactor;
    // Parameter specifying how many neighbors each candidate rectangle should have to retain it.
    int minNeighbors;
}CascadeProperties;

typedef struct StructCascade
{
    cv::CascadeClassifier cascade;
    CascadeProperties properties;
    bool nonDefaultParameters;
    StructCascade()
    {
        nonDefaultParameters = false;
    }
}Cascade;

typedef struct HumanFace
{
    cv::Rect leftEye;
    cv::Rect rightEye;
    cv::Rect nose;
    cv::Rect mouth;

    bool leftEyeFound;
    bool rightEyeFound;
    bool noseFound;
    bool mouthFound;
    HumanFace()
    {
        leftEyeFound = false;
        rightEyeFound = false;
        noseFound = false;
        mouthFound = false;
    }
}HumanFace;

#pragma pack(pop)

class FrokFaceDetector : public FaceDetectorAbstract
{
private:
    std::map <EnumCascades, Cascade>    cascades;
    cv::Ptr<cv::CLAHE>                  normalizerClahe;
    double                              aligningScaleFactor;
    cv::Size                            faceSize;
    cv::Size                            defaultImageSize;

public:
    FrokFaceDetector();
    ~FrokFaceDetector();

    FrokResult SetTargetImage(const char *imagePath, bool dontResize = false);
    FrokResult SetTargetImage(cv::Mat &image);
    FrokResult GetFacesFromPhoto(std::vector< cv::Rect > &faces);
    FrokResult GetFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages);
    FrokResult GetNormalizedFaceImages(std::vector< cv::Rect > &coords, std::vector< cv::Mat > &faceImages);

    FrokResult SetCascadeParameters(EnumCascades cascade, CascadeProperties params);
    FrokResult SetDefaultCascadeParameters(EnumCascades cascade, cv::Mat &imageWithObjects);
private:
    FrokResult AlignFaceImage(cv::Rect faceCoords, const cv::Mat &processedImage, cv::Mat &alignedFaceImage);
    FrokResult GetHumanFaceParts(cv::Mat &image, HumanFace *faceParts);
    FrokResult RemoveDrowbackFrokImage(cv::Mat &image);
    FrokResult ResizeFaceAndImage(cv::Mat &targetImage, cv::Rect &faceCoords);
};
#endif //C++

#ifdef __cplusplus
extern "C"{
#endif //C++

void* frokFaceDetectorAlloc();
void frokFaceDetectorDealloc(void* instance);
#ifdef __cplusplus
}
#endif //C++

#endif
