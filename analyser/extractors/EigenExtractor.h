#pragma once
#include "FeatureExtractor.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


using namespace std;
using namespace cv;


class EigenExtractor :
	public FeatureExtractor
{
public:
	EigenExtractor(string xmlDataFile);
	EigenExtractor(int eigenCount = 100);
	~EigenExtractor();


	void createFaceSpace(string trainDataFile);

	void load(string xmlDataFile);
	void save(string xmlDataFile);

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();
	
	string getName();
	
	void* createType(string &typeId);

private:
	Mat readFile(string trainDataFile);
	void toGrayscale(Mat& src, Mat& dst);
	

	int eigenfacesCount;
	cv::PCA pca;

	float mean;
	float std_deviation;
	float max;
	float min;
};

