#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/example.h>
#include "IClassifier.h"
#include "ezexample.h"
#include <iostream>     // std::cout
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand

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
	
	void importToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	void importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	
	float importToVowpalFormatTest(cv::Mat testData);
private:

	void shuffleTrainingData(cv::Mat& trainData, cv::Mat& trainLabels);
	vw* vwModel;
	vw* vwTest;
	std::map<int,int> initLabelToVowpalLabel;
	std::map<int,int> vowpalLabelToinitLabel;
	int numberOfClasses;
	bool readyToTrain;
};

