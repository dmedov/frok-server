#pragma once
class SiftDetection
{
public:
	Mat findDescriptors(IplImage *, char*);
	int matchDescriptors(Mat, Mat);
};

