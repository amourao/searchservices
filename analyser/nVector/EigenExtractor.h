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


class EigenExtractor :
	public FeatureExtractor
{
public:
	EigenExtractor();
	EigenExtractor(string& type);
	EigenExtractor(string& type, map<string,string>& params);

	void* createType(string &typeId);
	void* createType(string &typeId, map<string,string>& params);

	~EigenExtractor();

	void createFaceSpace(string trainDataFile);

	void load(string xmlDataFile);
	void save(string xmlDataFile);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();



private:
	Mat readFile(string trainDataFile);

    string type;

	int eigenfacesCount;
	cv::PCA pca;

	float mean;
	float std_deviation;
	float max;
	float min;
};

