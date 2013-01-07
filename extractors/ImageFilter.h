#pragma once

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

class ImageFilter
{
public:
	
	virtual ~ImageFilter()  {}

	virtual void applyFilter(Mat& src, Mat& dst) = 0;
};

