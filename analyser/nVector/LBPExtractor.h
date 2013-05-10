#pragma once

#include "FeatureExtractor.h"
#include "lbp.h"
#include "../FactoryAnalyser.h"

class LBPExtractor :
	public FeatureExtractor
{
public:
	LBPExtractor();
	LBPExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion);
	~LBPExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();

	string getName();
	
	void* createType(string &typeId);

private:
	int binCount;
	int horizontalDivisions;
	int verticalDivisions;
	bool useCenterRegion;
};

