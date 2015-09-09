#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>     // std::cout
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand
#include "../nTag/IClassifier.h"

using namespace std;

class TrainTestFeaturesTools
{
public:

	TrainTestFeaturesTools(cv::Mat data, cv::Mat labels, vector<IClassifier*> classifiers);
	TrainTestFeaturesTools(cv::Mat data, cv::Mat labels, cv::Mat dataTest, cv::Mat labelsTest, vector<IClassifier*> classifiers);
	~TrainTestFeaturesTools();

	cv::Mat getTrainingData();
	cv::Mat getTestData();

	cv::Mat getTrainingLabels();
	cv::Mat getTestLabels();

	string crossValidateAll(int numberOfDivisions);
	string testAll();

	string crossValidate(string name,int numberOfDivisions);
	string test(string name);

	void splitDataForTest(double ratio);

private:

	void test(cv::Mat trainData, cv::Mat trainLabels, cv::Mat testData, cv::Mat testLabels, IClassifier* classifier, vector<int>& correctGuesses, vector<int>& falsePositives, cv::Mat& confusionMatrix);
	string resultsToString(vector<int>& correctGuesses, vector<int>& falsePositives, cv::Mat& confusionMatrix);
	void divideByClass(cv::Mat trainData, cv::Mat trainLabels, double numberOfDivisions, int currentDivision, cv::Mat& newTrainData, cv::Mat& newTrainLabels, cv::Mat& testData, cv::Mat& testLabels);

	IClassifier* getClassifier(string name);

	cv::Mat data;
	cv::Mat labels;

	cv::Mat dataTest;
	cv::Mat labelsTest;

	vector<IClassifier*> classifiers;
};
