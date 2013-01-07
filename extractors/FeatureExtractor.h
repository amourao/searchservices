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

	//TODO: use functions from MatrixTools.h
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

	void extractFeatures(string& filename, Mat& dst){
		Mat src = imread(filename);
		extractFeatures(src,dst);	
	}
	
	void extractFeatures(string& filename, vector<vector<float>>& features){
		Mat dst;
		extractFeatures(filename,dst);
		
		for(int i = 0; i < dst.rows; i++){
			float* array = (float*)dst.row(i).data;
			features.at(i) = vector<float>(array, array + sizeof(float)*dst.cols);
		}
	}

	void extractFeatures(vector<float>& src, vector<vector<float>>& features){
		Mat srcMat(src,true);
		srcMat = srcMat.reshape(1,1);
		Mat dst;
		extractFeatures(srcMat,dst);
		for(int i = 0; i < dst.rows; i++){
			float* array = (float*)dst.row(i).data;
			features.at(i) = vector<float>(array, array + sizeof(float)*dst.cols);
		}

	}


};

