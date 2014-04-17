#pragma once
class DescriptorDetection
{
public:
	Mat findDescriptors(Mat, char*, bool);
	int matchDescriptors(Mat, Mat);
};

