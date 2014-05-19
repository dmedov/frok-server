#pragma once
class DescriptorDetection
{
public:
	Mat findDescriptors(Mat &face, char* name, bool b);
	int matchDescriptors(Mat &ffd, Mat &bfd);
};

