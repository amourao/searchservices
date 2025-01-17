#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>

#include "../tools/MatrixTools.h"
#include "../IAnalyser.h"
#include "../../dataModel/IDataModel.h"
#include "../../dataModel/NVector.h"
#include "../../commons/StringTools.h"



#define VECTOR_FEATURES_NAME "VectorFeatures"

using namespace cv;
using namespace std;

class FeatureExtractor: public IAnalyser
{
public:

	virtual ~FeatureExtractor()  {}

	virtual int getFeatureVectorSize() = 0;
	virtual string getName() = 0;

	virtual void* createType(string &typeId) = 0;

	virtual void extractFeatures(string filename, vector<float>& features){
		Mat src = imread(filename);
		Mat dst;
		extractFeatures(src,dst);
		float* array = (float*)dst.data;
		features = vector<float>(array, array + dst.cols*dst.rows);
	}

	virtual void extractFeaturesMulti(string filename, vector<vector<float> >& features){
		vector<string> filenames = StringTools::split(filename, ',');
		for (uint i = 0; i < filenames.size(); i++){
			vector<float> featuresSingle;
			extractFeatures(filenames.at(i),featuresSingle);
			features.push_back(featuresSingle);
		}

	}

	IDataModel::type getType(){
		return IDataModel::NVECTOR;
	}

	IDataModel* getFeatures(string name){
		vector<float> features;
		extractFeatures(name, features);
		NVector *vector = new NVector(name,getName(),features);
		return vector;
	}

	virtual void extractFeatures(string filename, vector<vector<float> >& features){
		Mat dst;
		extractFeatures(filename,dst);
		MatrixTools::matToVectors(dst,features);
	}
	//Extra functions (not necessary for now)
	virtual void extractFeatures(vector<float>& src, vector<float>& dst){
		Mat srcMat(src,true);
		srcMat = srcMat.reshape(1,1);
		Mat dstMat;
		extractFeatures(srcMat,dstMat);
		MatrixTools::matToVector(dstMat,dst);
	}

	virtual void extractFeatures(string filename, Mat& dst){
		Mat src = imread(filename);
		extractFeatures(src,dst);
	}

	virtual void extractFeatures(vector<float>& src, vector<vector<float> >& features){
		Mat srcMat(src,true);
		srcMat = srcMat.reshape(1,1);
		Mat dst;
		extractFeatures(srcMat,dst);
		MatrixTools::matToVectors(dst,features);
	}

	virtual void extractFeatures(cv::Mat& src, cv::Mat& dst){
        arma::fmat srcFMat, dstFMat;
        cv::Mat dstTmp;
        MatrixTools::matToFMat(src,srcFMat);
        extractFeatures(srcFMat,dstFMat);
        MatrixTools::fmatToMat(dstFMat,dstTmp);
        dstTmp.copyTo(dst);
    }

    virtual void extractFeatures(arma::fmat& src, arma::fmat& dst){
        cv::Mat srcFMat, dstFMat;
        MatrixTools::fmatToMat(src,srcFMat);
        extractFeatures(srcFMat,dstFMat);
        MatrixTools::matToFMat(dstFMat,dst);
    }


};

