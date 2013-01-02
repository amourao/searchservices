#pragma once
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

class IClassifier
{
public:
	virtual ~IClassifier() {}
	
	virtual void train(Mat trainData, Mat trainLabels) = 0;
	virtual void test(Mat testData, Mat testLabels) = 0;
	virtual float classify( Mat query) = 0;
	
	
};

