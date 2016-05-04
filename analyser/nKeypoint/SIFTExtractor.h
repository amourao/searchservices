#pragma once

#include "KeypointFeatureExtractor.h"
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/xfeatures2d.hpp>
#include "../FactoryAnalyser.h"

#include <map>

using namespace std;
using namespace cv::xfeatures2d;

class SIFTExtractor :
	public KeypointFeatureExtractor
{
public:
	SIFTExtractor();
	SIFTExtractor(string& type);
	SIFTExtractor(string& type, map<string,string>& params);
	SIFTExtractor(int nFeatures, int nOctaveLayers, double contrastThreshold, double edgeThreshold, double sigma);
	~SIFTExtractor();

    void* createType(string &typeId);
    void* createType(string &typeId, map<string,string>& params);


	void extractFeatures(Mat& source,vector< cv::KeyPoint>& keypoints, Mat& features);

	int getDescriptorSize();

	string getName();



private:
    string type;
    Ptr<SiftFeatureDetector> detector;
    Ptr<SiftDescriptorExtractor> extractor;
};

