#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "IClassifier.h"
#include "../../commons/StringTools.h"
#include <cstdlib>      // std::rand, std::srand
#include <iostream>
#include <fstream>
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector
#include "../FactoryClassifier.h"

#ifndef MODEL_EXTENSION_VW
#define MODEL_EXTENSION_VW ".model"
#endif

#ifndef BASIC_VW_RANDOM_SIZE
#define BASIC_VW_RANDOM_SIZE 16
#endif

#ifndef TRAINDATA_EXTENSION_VW
#define TRAINDATA_EXTENSION_VW ".train.tmp"
#endif

#ifndef PREDICTION_EXTENSION_VW
#define PREDICTION_EXTENSION_VW ".test.tmp"
#endif

#ifndef PREDICTION_READ_EXTENSION_VW
#define PREDICTION_READ_EXTENSION_VW ".prediction.tmp"
#endif

#ifndef LABEL_FILE_EXTENSION_VW
#define LABEL_FILE_EXTENSION_VW ".labels"
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

	void* createType(string &typeId);

	bool save(string basePath);
	bool load(string basePath);
	
	//void importToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	//void importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	
	//float importToVowpalFormatTest(cv::Mat testData);
private:

	void importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels);
	void shuffleTrainingData(cv::Mat& trainData, cv::Mat& trainLabels);
	float predictFromFile(cv::Mat testData);

	void loadLabelMap(string basePath);
	void saveLabelMap(string basePath);

	std::map<int,int> initLabelToVowpalLabel;
	std::map<int,int> vowpalLabelToinitLabel;
	int numberOfClasses;
	
	string modelName;

};

