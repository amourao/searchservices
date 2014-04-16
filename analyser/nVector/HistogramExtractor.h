#pragma once

#include "FeatureExtractor.h"
#include "../FactoryAnalyser.h"

#define HUE_FEATURES 0
#define SAT_FEATURES 1
#define VAL_FEATURES 2


class HistogramExtractor :
	public FeatureExtractor
{
public:
	HistogramExtractor();
	HistogramExtractor(string& type);
	HistogramExtractor(string& type, map<string,string>& params);
	HistogramExtractor(int binCount);
	~HistogramExtractor();

    void* createType(string& type);
	void* createType(string& type, map<string,string>& params);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();



private:

    string type;

	void extractFeaturesSingle(Mat& src, Mat& dst, int channel);

	int binCount;
	int featureChannel;
};

