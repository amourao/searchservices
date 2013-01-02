#pragma once
#include "dalm.h"
#include "fista.h"
#include "omp.h"
#include "thresholding.h"
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

class SRClassifier: public IClassifier
{
public:
	SRClassifier();
	~SRClassifier();

	void train(Mat trainData, Mat trainLabels);

	void test(Mat testData, Mat testLabels);
	float classify( Mat query);
private:

	
};

