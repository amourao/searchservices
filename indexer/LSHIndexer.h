#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

#include "IIndexer.h"
#include "FactoryIndexer.h"

#include "e2lsh/headers.h"
#include "e2lsh/NearNeighbors.h"

using namespace std;
using namespace cv;


#ifndef INDEX_DATA_EXTENSION_E2LSH
#define INDEX_DATA_EXTENSION_E2LSH ".xml"
#endif


#ifndef DEFAULT_MEMORY_MAX_AVAILABLE_E2LSH
#define DEFAULT_MEMORY_MAX_AVAILABLE_E2LSH 1073741824
#endif

class LSHIndexer: public IIndexer {

public:

	LSHIndexer();
	LSHIndexer(const string& type);
	LSHIndexer(const string& type, map<string,string>& params);
	~LSHIndexer();

	void* createType(string &typeId);
	void* createType(string &typeId, map<string,string>& params);

    void train(cv::Mat featuresTrain,cv::Mat featuresValidationI,cv::Mat featuresValidationQ);
	void indexWithTrainedParams(cv::Mat features);
    void index(cv::Mat features);

	std::pair<vector<float>,vector<float> > knnSearchId(const cv::Mat name, const int n);
	std::pair<vector<float>,vector<float> > radiusSearchId(const cv::Mat name, const double radius, const int n);
	bool save(string basePath);
	bool load(string basePath);

	string getName();

private:

    PPointT* matToPPointT(const Mat& input);
    Mat pPointsTToMat(const PPointT *(&input), int d, int n);
    vector<float> pPointsTToIndeces(PPointT *(&input), int n);
    vector<float> pPointsTToDists(PPointT *(&input), PPointT query, int d, int n);
    void sortFinalCandidates(PPointT *result, PPointT queryPoint, vector<float>& indicesFloat, vector<float>& dists, int d, int n, int newN);

	string typeId;
	cv::Mat indexData;
	map<string,string> paramsB;

	float oneMinusDelta;
	float r;
	float trainValSplit;
	PRNearNeighborStructT indexer;
	Mat featuresSplit;
	Mat validationSplit;
	long memoryUpperBound;

	RNNParametersT learnedParams;
};