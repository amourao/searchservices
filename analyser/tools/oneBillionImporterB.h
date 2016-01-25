#pragma once

#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <map>

#include <cstdlib>

#include "IBinImporter.h"

class oneBillionImporterB: public IBinImporter {

public:
	//not random, gets the first "numberOfRows" elements
	oneBillionImporterB();
    ~oneBillionImporterB();

	void readBin(std::string file, int numberOfRows, cv::Mat& features, long long offsetInRows = 0);
	void readBin(std::string file, int numberOfRows, arma::Mat<uchar>& features, long long offsetInRows = 0);
    void readBinSlow(std::string file, int numberOfRows, arma::Mat<uchar>& features, long long offsetInRows = 0);
	void readBin(std::string file, int numberOfRows, arma::fmat& features, long long offsetInRows = 0);
	void readTags(std::string file, int numberOfRows, cv::Mat& tags);

	void readBin(std::string filenamep, arma::Mat<uchar>& features, std::vector<uint>& buckets);
	void readBin(std::string filenamep, arma::Mat<int>& features, std::vector<uint>& buckets);
	void readBin(std::string filenamep, arma::Mat<float>& features, std::vector<uint>& buckets);

private:

};
