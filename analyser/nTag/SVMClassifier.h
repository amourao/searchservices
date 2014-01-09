#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "IClassifier.h"

#ifndef TRAINDATA_EXTENSION_SVM
#define TRAINDATA_EXTENSION_SVM ".xml"
#endif

using namespace std;

class SVMClassifier: public IClassifier
{
public:
	SVMClassifier();
	~SVMClassifier();

	void train(cv::Mat trainData, cv::Mat trainLabels);
	void test(cv::Mat testData, cv::Mat testLabels);
	float classify( cv::Mat query);

	bool save(string basePath);
	bool load(string basePath);

	string getName();
private:

	CvSVM* svm;
};

