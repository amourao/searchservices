#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "IClassifier.h"
#include <cstdlib>      // std::rand, std::srand
#include <iostream>
#include <fstream>
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector

#ifndef MODEL_EXTENSION
#define MODEL_EXTENSION ".model"
#endif

#ifndef TRAINDATA_EXTENSION
#define TRAINDATA_EXTENSION ".train.tmp"
#endif

#ifndef PREDICTION_EXTENSION
#define PREDICTION_EXTENSION ".test.tmp"
#endif

#ifndef PREDICTION_READ_EXTENSION
#define PREDICTION_READ_EXTENSION ".prediction.tmp"
#endif

#ifndef LABEL_FILE_EXTENSION
#define LABEL_FILE_EXTENSION ".labels"
#endif


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

	bool save(string basePath);
	bool load(string basePath);
	
	//void importToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	//void importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	
	//float importToVowpalFormatTest(cv::Mat testData);
private:

	void importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	void exportTxtToVowpalFormatClassification(cv::Mat testData);
	void shuffleTrainingData(cv::Mat& trainData, cv::Mat& trainLabels);
	float predictFromFile();

	void loadLabelMap(string basePath);
	void saveLabelMap(string basePath);

	std::map<int,int> initLabelToVowpalLabel;
	std::map<int,int> vowpalLabelToinitLabel;
	int numberOfClasses;
	
	string modelName;

};

