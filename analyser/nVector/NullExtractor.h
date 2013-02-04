#pragma once
#include "FeatureExtractor.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../FactoryAnalyser.h"

using namespace std;
using namespace cv;


class NullExtractor :
	public FeatureExtractor
{
public:
	NullExtractor();
	~NullExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();
	
	string getName();
	
	void* createType(string &typeId);

private:
};

