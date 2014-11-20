#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>

#include "IIndexer.h"
#include "FactoryIndexer.h"

#include "../commons/FileDownloader.h"
using namespace std;

#ifndef INDEX_DATA_EXTENSION_KNN
#define INDEX_DATA_EXTENSION_KNN ".xml"
#endif


#ifndef INDEX_FLANN_EXTENSION_KNN
#define INDEX_FLANN_EXTENSION_KNN ".flann"
#endif

class FlannkNNIndexer: public IIndexer {

public:

	FlannkNNIndexer();
	FlannkNNIndexer(string& type);
	FlannkNNIndexer(string& type, map<string,string>& params);
	~FlannkNNIndexer();

	void* createType(string &typeId);
	void* createType(string &typeId, map<string,string>& params);

    void train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ);
	void indexWithTrainedParams(cv::Mat& features);
	void index(cv::Mat& features);

	std::pair<vector<float>,vector<float> > knnSearchId(cv::Mat& name, int n, double search_limit);
	std::pair<vector<float>,vector<float> > radiusSearchId(cv::Mat& name, double radius, int n, double search_limit);

	bool save(string basePath);
	bool load(string basePath);

	int addToIndexLive(Mat& features);

	string getName();

private:

	string type;
	bool syncedWithDisk;
	cv::Mat indexData;
	cv::Mat indexDataUnsaved;
	flann::Index* flannIndexs;
	flann::IndexParams* flannParams;
	cvflann::flann_distance_t flannDistance;
};
