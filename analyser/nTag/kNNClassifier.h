#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <queue>
#include <opencv2/flann/flann.hpp>
#include "IClassifier.h"

using namespace cv;
using namespace std;

class kNNClassifier: public IClassifier
{
public:
	kNNClassifier();
	~kNNClassifier();

	void train(cv::Mat trainData, cv::Mat trainLabels);

	void test(cv::Mat testData, cv::Mat testLabels);
		
	float classify( cv::Mat query);
	
	float classify( cv::Mat query, int neighboursCount = 1);
	
	string getName();
private:

	cv::Mat trainLabels;
	cv::Mat trainData;
	flann::Index* flannIndex;
	int numberOfClasses;
};
