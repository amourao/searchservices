#pragma once
#include <opencv2/core/core.hpp>
#include <string>

using namespace cv;


class ImageSource
{
    public:
	virtual ~ImageSource() {}
	virtual Mat nextImage() = 0;
	virtual std::string getImageInfo() = 0;
	virtual bool isAvailable() = 0;
	virtual int getImageCount() = 0;
	virtual int getRemainingImageCount() = 0;
};
