#pragma once
class SiftDetection
{
public:
	Mat findDescriptors(IplImage *, char*, bool);
	int matchDescriptors(Mat, Mat);
};

