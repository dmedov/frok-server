#pragma once
class SiftDetection
{
public:
	Mat findDescriptors(IplImage *, char*);
private:
	void matchDescriptors(Mat, Mat);
};

