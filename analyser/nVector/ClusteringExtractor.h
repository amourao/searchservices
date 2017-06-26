#pragma once

#include <opencv2/flann/flann.hpp>

#include "FeatureExtractor.h"
#include "../FactoryAnalyser.h"
#include "../../commons/StringTools.h"

class ClusteringExtractor :
	public FeatureExtractor
{
public:

	ClusteringExtractor();
	ClusteringExtractor(string& type);
	ClusteringExtractor(string& type, map<string, string>& params);

	~ClusteringExtractor();

    void* createType(string &typeId);
	void* createType(string &typeId, map<string, string>& params);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();



private:
    string type;

	int descsize;

	flann::Index flannIndex;
	

};

