#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class IBinImporter {

public:

	virtual void readBin(std::string file, int numberOfRows, cv::Mat& features, long long offsetInRows = 0) = 0;
	virtual void readTags(std::string file, int numberOfRows, cv::Mat& tags) = 0;

};
