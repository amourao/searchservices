#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include "../analyser/tools/MatrixTools.h"

#include "../commons/factory/Factory.h"

using namespace std;

#ifndef INDEXER_BASE_SAVE_PATH
#define INDEXER_BASE_SAVE_PATH "./indexer/data/"
#endif

class IIndexer: public FactoryMethod {

public:

	virtual ~IIndexer(){}
	
	virtual void* createType(string &typeId) = 0;
	
	virtual vector<float> knnSearchId(string name, int n) = 0;
	virtual vector<string> knnSearchName(string name, int n) = 0;

	virtual vector<float> radiusSearchId(string name, double radius) = 0;
	virtual vector<string> radiusSearchName(string name, double radius) = 0;

	virtual void index(cv::Mat features) = 0;
	virtual bool save(string basePath) = 0;
	virtual bool load(string basePath) = 0;

	virtual string getName() = 0;
};


