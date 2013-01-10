#pragma once
#include "ImageFilter.h"

#define ILLUMINATION_THRESHOLD 50

class IlluminationCorrectionFilter : public ImageFilter
{
public:

	IlluminationCorrectionFilter(int width, int height, cv::Rect right= cv::Rect(0,40,27,27), cv::Rect left = cv::Rect(65,40,27,27));
	IlluminationCorrectionFilter(cv::Rect right= cv::Rect(0,40,27,27), cv::Rect left = cv::Rect(65,40,27,27));
	~IlluminationCorrectionFilter();

	void createIlluminationFilters(int width, int height);
	void applyFilter(Mat& src, Mat& dst);

private:

	Mat illuminationFilterBrightFirst;
	Mat illuminationFilterDarkFirst;
	cv::Rect illumBaseLeft;
	cv::Rect illumBaseRight;
	int width; 
	int height;

};

