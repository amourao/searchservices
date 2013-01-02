#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

class FeatureExtractor
{
public:
	
	virtual ~FeatureExtractor()  {}

	virtual void extractFeatures(Mat& src, Mat& dst) = 0;
	virtual int getFeatureVectorSize() = 0;

	void extractFeatures(string& filename, vector<float>& features){
		Mat src = imread(filename);
		Mat dst;
		extractFeatures(src,dst);
		float* array = (float*)dst.data;
		features = vector<float>(array, array + sizeof(float)*dst.cols*dst.rows);
	}

	void extractFeatures(vector<float>& src, vector<float>& dst){
		Mat srcMat(src,true);
		srcMat = srcMat.reshape(1,1);
		Mat dstMat;
		extractFeatures(srcMat,dstMat);
		float* array = (float*)dstMat.data;
		dst = vector<float>(array, array + sizeof(float)*dstMat.cols*dstMat.rows);
	}


};

