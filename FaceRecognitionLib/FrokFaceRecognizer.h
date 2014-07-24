#ifndef FrokFaceRecognizer_H
#define FrokFaceRecognizer_H

// include dependencies
#include "FaceUserModel.h"
#include "faceCommonLib.h"

class FrokFaceRecognizer
{
public:
private:
public:
    FrokResult SetTargetImage(IplImage *image);
    FrokResult SetTargetImage(char *imagePath);

    FrokResult CompareToModel(IplImage *comparationResult, FaceUserModel *model);
    double GetSimilarity(IplImage *comparationResult);
private:
};

#endif // FrokFaceRecognizer_H
