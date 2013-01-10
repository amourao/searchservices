#pragma once

#include "FeatureExtractor.h"


#define HUE_FEATURES 0
#define SAT_FEATURES 1
#define VAL_FEATURES 2


class HistogramExtractor :
	public FeatureExtractor
{
public:
	HistogramExtractor(int binCount);
	~HistogramExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();

	string getName();
	
	void* createType(string &typeId);
	
private:
	
	void extractFeaturesSingle(Mat& src, Mat& dst, int channel);

	int binCount;
	int featureChannel;
};

