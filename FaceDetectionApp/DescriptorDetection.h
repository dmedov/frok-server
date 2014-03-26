#pragma once
class DescriptorDetection
{
public:
	Mat findDescriptors(IplImage *, char*, bool);
	int matchDescriptors(Mat, Mat);
};

