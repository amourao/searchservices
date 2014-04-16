#pragma once

#include "FeatureExtractor.h"
#include "lbp.h"
#include "../FactoryAnalyser.h"

class LBPExtractor :
	public FeatureExtractor
{
public:
	LBPExtractor();
	LBPExtractor(string& type);
	LBPExtractor(string& type, map<string, string>& params);
	LBPExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion);
	~LBPExtractor();

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();

	void* createType(string& type);
	void* createType(string& type, map<string, string>& params);

private:

    string type;
	int binCount;
	int horizontalDivisions;
	int verticalDivisions;
	bool useCenterRegion;
};

