#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "ImageSource.h"
class SingleImageSource :
	public ImageSource
{
public:
	SingleImageSource(std::string filename);
	~SingleImageSource();

	cv::Mat nextImage();
	std::string getImageInfo();
	bool isAvailable();
	int getImageCount();
	int getRemainingImageCount();

private:

	cv::Mat image;

};

