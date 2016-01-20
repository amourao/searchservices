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

#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>

#include <armadillo>

using namespace std;


class ANdOMPExtractor :
	public FeatureExtractor
{
public:
	ANdOMPExtractor();
	ANdOMPExtractor(string& type, map<string, string>& params);
	ANdOMPExtractor(arma::fmat& dictionary, int max_iters, double eps);
	~ANdOMPExtractor();

	void extractFeaturesBatchSparseConstrained(arma::fmat& src,arma::fmat& G, arma::fmat& dst);
	void extractFeatures(arma::fmat& src, arma::fmat& dst);

	int getFeatureVectorSize();

	string getName();

	void* createType(string &typeId);
	void* createType(string& type, map<string, string>& params);

	void changeDictionary(arma::fmat new_dictionary);

    arma::fmat D;
private:


    arma::Row<long> distribution;
    string type;

    uint max_iters;
    double eps;
};

