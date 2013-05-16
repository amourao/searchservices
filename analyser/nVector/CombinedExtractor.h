#pragma once

#include "FeatureExtractor.h"
#include "SegmentedHistogramExtractor.h"
#include "LBPExtractor.h"
#include <opencv2/highgui/highgui.hpp>
#include "../FactoryAnalyser.h"


using namespace cv;
using namespace std;

class CombinedExtractor :
	public FeatureExtractor
{
public:
	CombinedExtractor();
	CombinedExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion);
	~CombinedExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();

	string getName();
	
	void* createType(string &typeId);

private:
	SegmentedHistogramExtractor* she;
	LBPExtractor* lbpe;
};

