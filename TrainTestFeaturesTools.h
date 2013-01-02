#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <vector>
#include <sstream>
#include "classifiers/IClassifier.h"

using namespace std;

class TrainTestFeaturesTools
{
public:
	
	TrainTestFeaturesTools(cv::Mat data, cv::Mat labels, vector<IClassifier*> classifiers);
	TrainTestFeaturesTools(cv::Mat data, cv::Mat labels, cv::Mat dataTest, cv::Mat labelsTest, vector<IClassifier*> classifiers);
	~TrainTestFeaturesTools();
	
	Mat getTrainingData();
	Mat getTestData();
	
	Mat getTrainingLabels();
	Mat getTestLabels();

	string crossValidateAll(int numberOfDivisions);
	string testAll();
	
	string crossValidate(string name,int numberOfDivisions);
	string test(string name);
	
private:

	string test(Mat trainData, Mat trainLabels, Mat testData, Mat testLabels, IClassifier* classifier, vector<int>& correctGuesses, vector<int>& falsePositives, Mat& confusionMatrix);
	string resultsToString(vector<int>& correctGuesses, vector<int>& falsePositives, Mat& confusionMatrix);
	void divideByClass(Mat trainData, Mat trainLabels, int numberOfDivisions, int currentDivision,Mat& newTrainData,Mat& newTrainLabels, Mat& testData, Mat& testLabels);
	
	IClassifier* getClassifier(string name);

	cv::Mat data;
	cv::Mat labels;
	
	cv::Mat dataTest;
	cv::Mat labelsTest;
	
	vector<IClassifier*> classifiers;
};
