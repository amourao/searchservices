#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "IClassifier.h"
#include "../FactoryClassifier.h"

#ifndef TRAINDATA_EXTENSION_SVM
#define TRAINDATA_EXTENSION_SVM ".xml"
#endif

using namespace std;

class SVMClassifier: public IClassifier
{
public:
	SVMClassifier();
	SVMClassifier(string& type);
	~SVMClassifier();

	void train(cv::Mat trainData, cv::Mat trainLabels);
	void test(cv::Mat testData, cv::Mat testLabels);
	float classify( cv::Mat query);

	bool save(string basePath);
	bool load(string basePath);

	void* createType(string &typeId);

	string getName();
private:

	CvSVM* svm;
};

