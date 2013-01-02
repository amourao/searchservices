#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <queue>
#include <opencv2/flann/flann.hpp>
#include "IClassifier.h"

using namespace cv;
using namespace std;

class kNNClassifier
{
public:
	kNNClassifier();
	~kNNClassifier();

	void train(Mat trainData, Mat trainLabels);

	void test(Mat testData, Mat testLabels);
	float classify( Mat query, int neighboursCount = 1);
private:

	Mat trainLabels;
	flann::Index* flannIndex;
};
