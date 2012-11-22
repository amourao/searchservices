#pragma once
#include "imagesource.h"
#include <opencv2/opencv.hpp>

using namespace std;

class VideoSource :
	public ImageSource
{
public:
	VideoSource(string filename);
	~VideoSource();

	Mat nextImage();
	string getImageInfo();
	bool isAvailable();

private:
	VideoCapture capture;
};

