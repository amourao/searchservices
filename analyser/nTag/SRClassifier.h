#pragma once
#include "dalm.h"
#include "fista.h"
#include "omp.h"
#include "thresholding.h"
#include "IClassifier.h"
#include <opencv2/highgui/highgui.hpp>

using namespace std;

class SRClassifier: public IClassifier
{
public:
	SRClassifier();
	~SRClassifier();

	void train(cv::Mat trainData, cv::Mat trainLabels);
	void test(cv::Mat testData, cv::Mat testLabels);
	float classify( cv::Mat query);
	
	void train(arma::fmat trainData, arma::fmat trainLabels);
	void test(arma::fmat testData, arma::fmat testLabels);
	float classify(arma::fmat query, double* error = NULL, arma::fmat* recErrors = NULL);
	string getName();
	
private:

	arma::fmat trainData;
	arma::fvec labelsCute;
	l1min::FISTA* omp;
	
	int numberOfClasses;
};

