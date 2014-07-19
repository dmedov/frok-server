#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

// include dependencies
#include "FaceUserModel.h"
#include "faceCommonLib.h"

class FaceRecognizer
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

#endif // FACERECOGNIZER_H
