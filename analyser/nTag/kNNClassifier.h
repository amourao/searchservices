#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <queue>
#include <opencv2/flann/flann.hpp>
#include "IClassifier.h"
#include "../FactoryClassifier.h"

using namespace cv;
using namespace std;


#ifndef TRAINDATA_EXTENSION_KNN
#define TRAINDATA_EXTENSION_KNN ".xml"
#endif

#ifndef FLANN_EXTENSION_KNN
#define FLANN_EXTENSION_KNN ".flann"
#endif


class kNNClassifier: public IClassifier
{
public:
	kNNClassifier();
	kNNClassifier(string& type);
	kNNClassifier(string& type, map<string, string>& params);
	~kNNClassifier();

    void* createType(string &typeId);
	void* createType(string &typeId, map<string, string>& params);

	void train(cv::Mat trainData, cv::Mat trainLabels);

	void test(cv::Mat testData, cv::Mat testLabels);

	float classify( cv::Mat query);

	float classify( cv::Mat query, int neighboursCount);

	string getName();



	bool save(string basePath);
	bool load(string basePath);
private:

	cv::Mat trainLabels;
	cv::Mat trainData;
	flann::Index* flannIndex;
	int numberOfClasses;
	string type;
};
