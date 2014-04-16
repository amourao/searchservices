#pragma once

#include "KeypointFeatureExtractor.h"
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include "../FactoryAnalyser.h"

#include <map>
/*
 * [Bay06]
 * Bay, H. and Tuytelaars, T. and Van Gool, L.
 * “SURF: Speeded Up Robust Features”,
 * 9th European Conference on Computer Vision, 2006
 *
 */

using namespace std;
using namespace cv;

class SURFExtractor :
	public KeypointFeatureExtractor
{
public:
	SURFExtractor();
	SURFExtractor(string& type);
	SURFExtractor(string& type, map<string,string>& params);
	SURFExtractor(double hessianThreshold, int nOctaves=4, int nOctaveLayers=2, bool extended=true, bool upright=false);
	~SURFExtractor();

    void* createType(string &typeId);
    void* createType(string &typeId, map<string,string>& params);


	void extractFeatures(Mat& source,vector< cv::KeyPoint>& keypoints, Mat& features);

	int getDescriptorSize();

	string getName();



private:
    string type;
	SurfFeatureDetector detector;
	SurfDescriptorExtractor extractor;
};

