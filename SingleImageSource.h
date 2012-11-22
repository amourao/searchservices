#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "imagesource.h"
class SingleImageSource :
	public ImageSource
{
public:
	SingleImageSource(string filename);
	~SingleImageSource();

	Mat nextImage();
	string getImageInfo();
	bool isAvailable();

private:

	Mat image;

};

