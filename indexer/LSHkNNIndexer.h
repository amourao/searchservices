#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>

#include "IIndexer.h"
#include "FactoryIndexer.h"

using namespace std;
using namespace cv;


#ifndef INDEX_DATA_EXTENSION_KNN
#define INDEX_DATA_EXTENSION_KNN ".xml"
#endif


#ifndef INDEX_FLANN_EXTENSION_KNN
#define INDEX_FLANN_EXTENSION_KNN ".flann"
#endif

class LSHkNNIndexer: public IIndexer {

public:

	LSHkNNIndexer();
	LSHkNNIndexer(string& type, map<string,string> params);
	~LSHkNNIndexer();
	
	void* createType(string &typeId);
	
	void index(cv::Mat features);

	vector<std::pair<float,float> > knnSearchId(cv::Mat name, int n);
	vector<std::pair<string,float> > knnSearchName(cv::Mat name, int n);

	vector<std::pair<float,float> > radiusSearchId(cv::Mat name, double radius, int n);
	vector<std::pair<string,float> > radiusSearchName(cv::Mat name, double radius, int n);

	bool save(string basePath);
	bool load(string basePath);

	string getName();

private:

	string type;
	cv::Mat indexData;
	flann::Index* flannIndexs;
	flann::IndexParams* flannParams;	
	cvflann::flann_distance_t flannDistance;
	map<string,string> paramsB;

	FlannBasedMatcher* matcher;
};
