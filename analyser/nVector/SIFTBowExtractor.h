#pragma once
#include "FeatureExtractor.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/ml.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../FactoryAnalyser.h"
#include "../tools/MatrixTools.h"

using namespace std;
using namespace cv;
using namespace cv::ml;
using namespace cv::xfeatures2d;

class SIFTBowExtractor :
	public FeatureExtractor
{
public:
	SIFTBowExtractor();
	SIFTBowExtractor(string& type);
	SIFTBowExtractor(string& type, map<string,string>& params);

	void* createType(string &typeId);
	void* createType(string &typeId, map<string,string>& params);

	~SIFTBowExtractor();

	void load(string xmlDataFile);
	void save(string xmlDataFile);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();


private:
	void loadVocabulary(string trainDataFile);

    string type;

	Mat vocabulary;
	Ptr<FeatureDetector> detector;
	Ptr<BOWImgDescriptorExtractor> bowMatcher;

};

