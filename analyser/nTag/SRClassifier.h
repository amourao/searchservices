#pragma once
#include <l1min/dalm.h>
#include <l1min/fista.h>
#include <l1min/omp.h>
#include <l1min/thresholding.h>
#include "IClassifier.h"
#include <opencv2/highgui/highgui.hpp>
#include "../FactoryClassifier.h"

using namespace std;


#ifndef TRAINDATA_EXTENSION_SR
#define TRAINDATA_EXTENSION_SR ".xml"
#endif



class SRClassifier: public IClassifier
{
public:

    SRClassifier();
	SRClassifier(string& type);
	SRClassifier(string& type, map<string, string>& params);
	~SRClassifier();

    void* createType(string &typeId);
	void* createType(string &typeId, map<string, string>& params);

	void train(cv::Mat trainData, cv::Mat trainLabels);
	void test(cv::Mat testData, cv::Mat testLabels);
	float classify( cv::Mat query);

	bool save(string basePath);
	bool load(string basePath);

	void train(arma::fmat trainData, arma::fmat trainLabels);
	void test(arma::fmat testData, arma::fmat testLabels);
	float classify(arma::fmat query, double* error = NULL, arma::fmat* recErrors = NULL);
	string getName();

	void changeLabels(arma::fmat trainLabels);
private:

    string type;
	arma::fmat trainData;
	arma::fvec labelsCute;
	l1min::FISTA* omp;
	l1min::FISTA::option_type opt;

	int numberOfClasses;
};


