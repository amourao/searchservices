#pragma once

#include "FeatureExtractor.h"
#include "HistogramExtractor.h"
#include <iostream>

using namespace std;

class SegmentedHistogramExtractor :
	public FeatureExtractor
{
public:
	SegmentedHistogramExtractor(int binCount, int horizontalDivisions, int verticalDivisions);
	~SegmentedHistogramExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();


private:
	
	Ptr<HistogramExtractor> singleHistogramExtractor;
	int binCount;
	int horizontalDivisions;
	int verticalDivisions;
};

