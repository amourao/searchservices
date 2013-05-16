#pragma once

#include "FeatureExtractor.h"
#include <iostream>
#include "../FactoryAnalyser.h"
#include <cstdlib>


using namespace std;

class LireExtractor :
	public FeatureExtractor
{
public:
	
	LireExtractor();
	LireExtractor(string type);
	~LireExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	void extractFeatures(string filename, vector<float>& features);
	
	int getFeatureVectorSize();

	string getName();
	
	void* createType(string &typeId);
private:
	
	string type;
};

