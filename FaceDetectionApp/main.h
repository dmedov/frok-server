#pragma once
class Main
{
public:
	Mat findDescriptors(IplImage *, char*);
private:
	void matchDescriptors(Mat, Mat);
};
