#pragma once

#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <map>
#include <cstdlib>


class tinyImageImporter {

public:
	//not random, gets the first "numberOfRows" elements
	static void readBin(std::string file, int numberOfRows, cv::Mat& features, long long offsetInRows = 0);
	static void readTags(std::string file, int numberOfRows, cv::Mat& tags);

private:

	const static int dimensions = 384;

};
