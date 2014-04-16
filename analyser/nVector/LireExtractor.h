#pragma once

#include "FeatureExtractor.h"
#include <iostream>
#include "../FactoryAnalyser.h"
#include "../../commons/StringTools.h"
#include <cstdlib>


using namespace std;

class LireExtractor :
	public FeatureExtractor
{
public:

	LireExtractor();
	LireExtractor(string& type);
	LireExtractor(string& type, map<string ,string>& params);
	~LireExtractor();

    void* createType(string &typeId);
    void* createType(string &typeId, map<string ,string>& params);


	void extractFeatures(Mat& src, Mat& dst);
	void extractFeatures(string filename, vector<float>& features);

	int getFeatureVectorSize();

	string getName();




private:

	string type;
	string algorithm;

};

