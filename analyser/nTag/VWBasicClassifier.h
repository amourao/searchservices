#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <vowpalwabbit/vw.h>
#include "IClassifier.h"
#include "ezexample.h"

using namespace std;

class VWBasicClassifier: public IClassifier
{
public:
	VWBasicClassifier();
	~VWBasicClassifier();

	void train(cv::Mat trainData, cv::Mat trainLabels);

	void test(cv::Mat testData, cv::Mat testLabels);
	float classify( cv::Mat query);
	string getName();
	
	void importToVowpalFormat(ezexample& ez, cv::Mat trainData, cv::Mat trainLabels);
	float importToVowpalFormatTest(ezexample& ez, cv::Mat testData);
private:

	vw* vwModel;
};

