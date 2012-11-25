#pragma once

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

class FeatureExtractor
{
public:
	
	virtual ~FeatureExtractor()  {}

	virtual void extractFeatures(Mat& src, Mat& dst) = 0;
	virtual int getFeatureVectorSize() = 0;
};

