#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <vowpalwabbit/vw.h>
#include <vowpalwabbit/example.h>
#include "IClassifier.h"
#include "ezexample.h"
#include <cstdlib>      // std::rand, std::srand
#include <iostream>
#include <fstream>
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector

using namespace std;

class VWBasicClassifier: public IClassifier
{
public:
	VWBasicClassifier();
	VWBasicClassifier(string _modelName);
	~VWBasicClassifier();

	void train(cv::Mat trainData, cv::Mat trainLabels);

	void test(cv::Mat testData, cv::Mat testLabels);
	float classify( cv::Mat query);
	string getName();
	
	//void importToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	//void importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	
	//float importToVowpalFormatTest(cv::Mat testData);
private:

	void importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	void exportTxtToVowpalFormatClassification(cv::Mat testData);
	void shuffleTrainingData(cv::Mat& trainData, cv::Mat& trainLabels);
	float predictFromFile();
	std::map<int,int> initLabelToVowpalLabel;
	std::map<int,int> vowpalLabelToinitLabel;
	int numberOfClasses;
	
	string modelName;

};

