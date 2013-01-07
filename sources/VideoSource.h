#pragma once
#include "ImageSource.h"
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
	int getImageCount();
	int getRemainingImageCount();

private:
	VideoCapture capture;
};

