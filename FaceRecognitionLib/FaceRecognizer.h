#ifndef FACERECOGNIZER_H
#define FACERECOGNIZER_H

// include dependencies
#include "FaceUserModel.h"
#include "../FaceCommonLib/faceCommonLib.h"

class FaceRecognizer
{
public:
private:
public:
    FrokResult SetTargetImage(IplImage *targetImage);
    FrokResult SetTargetImage(char *targetImagePath);

    FrokResult CompareToModel(IplImage *comparationResult, FaceUserModel *model);
    double GetSimilarity(IplImage *comparationResult);
private:
};

#endif // FACERECOGNIZER_H
