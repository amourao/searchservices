#pragma once


#include <utils/utils.h>

#include <l1min/omp.h>

#include "FeatureExtractor.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../FactoryAnalyser.h"
#include "../tools/MatrixTools.h"


using namespace std;
using namespace arma;
using namespace l1min;


class SRExtractor :
	public FeatureExtractor
{
public:
	SRExtractor();
	SRExtractor(string& type, map<string, string>& params);
	~SRExtractor();

	void extractFeatures(cv::Mat& src, cv::Mat& dst);
	void extractFeatures(arma::fmat& src, arma::fmat& dst);

	int getFeatureVectorSize();

	string getName();

	void* createType(string &typeId);
	void* createType(string& type, map<string, string>& params);

private:

    l1min::OMPSparseConstrained* omp;
    arma::fmat dict;
    string type;
};

