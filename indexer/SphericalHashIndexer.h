#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <boost/dynamic_bitset.hpp>

#include "IIndexer.h"
#include "FactoryIndexer.h"
#include "FactoryIndexer.h"


#include "./sphericalHashing/BinaryHash.h"
#include "./sphericalHashing/Evaluation.h"

using namespace std;
using namespace cv;


#ifndef INDEX_DATA_EXTENSION_SPHI
#define INDEX_DATA_EXTENSION_SPHI ".xml"
#endif

 struct compareVectDists{
     inline bool operator()(const pair<float,float>& pair1, const pair<float,float>& pair2){
     	if (pair1.second == pair2.second)
     		return pair1.first < pair2.first;//optional, sorts by id if equal
        return (pair1.second < pair2.second);
     }
 };

class SphericalHashIndexer: public IIndexer {

public:

	SphericalHashIndexer();
	SphericalHashIndexer(const string& type);
	SphericalHashIndexer(const string& type, map<string,string>& params);
	~SphericalHashIndexer();

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

    void matToPoints(const cv::Mat& input, Points& output);

    void Do_ZeroCentering(Points& in, float* center);

    void Undo_ZeroCentering(Points& in, float* center);

	string typeId;

    // dps: data points set
    // qps: query points set
    Points dps;


    LSH lsh;
    SphericalHashing sh;

    // nP: number of data points
    int nP, dims;

    float *dataCenter;

    bool isLSH;
    bool SHD;


    boost::dynamic_bitset<> *bCodeData;

    int dim;
	int bCodeLen;
	int trainSamplesCount;
	float incRatio;
	float overRatio;
	float epsMean;
	float epsStdDev;
	float examineRatio;
	int maxItr;

};
