#pragma once
#include "ImageSource.h"
#include <opencv2/opencv.hpp>

using namespace std;

class CamSource :
	public ImageSource
{
public:
	CamSource();
	CamSource(int cameraIndex);
	~CamSource();

	 Mat nextImage();
	 string getImageInfo();
	 bool isAvailable();

private:
	VideoCapture capture;

	void init(int cameraIndex);
};

