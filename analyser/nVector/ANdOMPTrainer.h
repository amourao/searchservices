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

#include "ANdOMPExtractor.h"

using namespace std;


class ANdOMPTrainer :
	public FeatureExtractor
{
public:
	ANdOMPTrainer();
	ANdOMPTrainer(string& type, map<string, string>& params);
	ANdOMPTrainer(ANdOMPExtractor _fe, int _n_iters, double _eps, uint _dimensions, bool withBias);
	~ANdOMPTrainer();

	void iterate();

	int getFeatureVectorSize();

	string getName();

	void train(arma::fmat& dictionary, arma::fmat& train, arma::fmat& validationI, arma::fmat& validationQ);


	void* createType(string &typeId);
	void* createType(string& type, map<string, string>& params);

	arma::fmat V;
	arma::fmat X;

	arma::fmat D_seed;
    arma::fmat D;

    arma::fmat G;
    arma::fmat Gamma;

    arma::Col<double> countAppearing;
    arma::Col<double> sumAppearing;

    arma::Col<double> lastCountAppearing;
    arma::Col<double> lastSumAppearing;

    string type;

    uint n_iters;
    uint dimensions;
    double eps;
    bool withBias;

    ANdOMPExtractor fe;
};

