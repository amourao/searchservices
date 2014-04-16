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
	SegmentedHistogramExtractor(string& type);
	SegmentedHistogramExtractor(string& type, map<string, string>& params);
	SegmentedHistogramExtractor(int binCount, int horizontalDivisions, int verticalDivisions, bool useCenterRegion = false);
	~SegmentedHistogramExtractor();

	void init(int binCount, int horizontalDivisions, int verticalDivisions, bool useCenterRegion);

	void* createType(string &typeId);
	void* createType(string& type, map<string, string>& params);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();


private:

    string type;

	Ptr<HistogramExtractor> singleHistogramExtractor;
	int binCount;
	int horizontalDivisions;
	int verticalDivisions;
	bool useCenterRegion;
};

