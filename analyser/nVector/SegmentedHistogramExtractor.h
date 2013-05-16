#pragma once

#include "FeatureExtractor.h"
#include "HistogramExtractor.h"
#include <iostream>
#include "../FactoryAnalyser.h"

using namespace std;

class SegmentedHistogramExtractor :
	public FeatureExtractor
{
public:
	
	SegmentedHistogramExtractor();
	SegmentedHistogramExtractor(int binCount, int horizontalDivisions, int verticalDivisions);
	SegmentedHistogramExtractor(int binCount, int horizontalDivisions, int verticalDivisions, bool useCenterRegion);
	~SegmentedHistogramExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();

	string getName();
	
	void* createType(string &typeId);
private:
	
	Ptr<HistogramExtractor> singleHistogramExtractor;
	int binCount;
	int horizontalDivisions;
	int verticalDivisions;
	bool useCenterRegion;
};

