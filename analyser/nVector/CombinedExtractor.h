#pragma once

#include "FeatureExtractor.h"
#include "../FactoryAnalyser.h"
#include "../../commons/StringTools.h"

class CombinedExtractor :
	public FeatureExtractor
{
public:

	CombinedExtractor();
	CombinedExtractor(string& type);
	CombinedExtractor(string& type, map<string, string>& params);

	CombinedExtractor(int _nblocks, int _nscale, vector<int> _orientations_per_scale, int _imageW, int _imageH);
	~CombinedExtractor();

    void init(int _nblocks, int _nscale, vector<int> _orientations_per_scale, int _imageW, int _imageH);

    void* createType(string &typeId);
	void* createType(string &typeId, map<string, string>& params);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();



private:
    string type;

	int descsize;

	vector<string> algorithmsType;
	vector<FeatureExtractor*> algorithms;


};

