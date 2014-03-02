#include "stdafx.h"
#include "SiftDetection.h"

//Сравнение дескрипторов (доделать)
void SiftDetection::matchDescriptors(Mat ffd, Mat bfd){
	FlannBasedMatcher matcher;
	vector<DMatch> matches;
	matcher.match(ffd, bfd, matches);

	double max_dist = 0; double min_dist = 2;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < ffd.rows; i++)	{
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	vector< DMatch > good_matches;
	int c = 0;
	for (int i = 0; i < ffd.rows; i++)	{
		if (matches[i].distance <= 2 * min_dist)	{
			good_matches.push_back(matches[i]);
			c++;
		}
	}

}

//Поиск дескрипторов и вывод изображений с найденными дескрипторами
Mat SiftDetection::findDescriptors(IplImage *face, char* name){
	//-- Этап 1. Нахождение ключевых точек.
	SiftFeatureDetector detector(400);
	vector<KeyPoint> keypoints;
	detector.detect(face, keypoints);
	Mat img_keypoints, descriptors_object;
	drawKeypoints(face, keypoints, img_keypoints, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

	//-- Этап 2. Вычисление дескрипторов.
	SiftDescriptorExtractor extractor;
	extractor.compute(face, keypoints, descriptors_object);

	imshow(name, img_keypoints);
	return descriptors_object;
}

