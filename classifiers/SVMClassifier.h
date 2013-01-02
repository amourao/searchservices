#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include "IClassifier.h"

using namespace cv;
using namespace std;

class SVMClassifier: public IClassifier
{
public:
	SVMClassifier();
	~SVMClassifier();

	void train(Mat trainData, Mat trainLabels);

	void test(Mat testData, Mat testLabels);
	float classify( Mat query);
private:

	CvSVM svm;
};

