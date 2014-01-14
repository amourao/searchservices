#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <armadillo>
#include "../tools/MatrixTools.h"


#ifndef CLASSIFIER_BASE_SAVE_PATH
#define CLASSIFIER_BASE_SAVE_PATH "./analyser/data/classifiers/"
#endif

using namespace std;

class IClassifier
{
public:
	virtual ~IClassifier() {}
	
	virtual void train(cv::Mat trainData, cv::Mat trainLabels) = 0;
	virtual void test(cv::Mat testData, cv::Mat testLabels) = 0;

	virtual bool save(string basePath) = 0;
	virtual bool load(string basePath) = 0;

	virtual float classify( cv::Mat query) = 0;
	virtual string getName() = 0;
	
	virtual void train(arma::fmat trainData, arma::fmat trainLabels){
		cv::Mat trainDataMat;
		cv::Mat trainLabelsMat;
		MatrixTools::fmatToMat(trainData, trainDataMat);
		MatrixTools::fmatToMat(trainLabels, trainLabelsMat);
		train(trainDataMat,trainLabelsMat);
	}
	
	virtual void test(arma::fmat testData, arma::fmat testLabels){
		cv::Mat testDataMat;
		cv::Mat testLabelsMat;
		MatrixTools::fmatToMat(testData, testDataMat);
		MatrixTools::fmatToMat(testLabels, testLabelsMat);
		test(testDataMat,testLabelsMat);
	}
	
	virtual float classify(arma::fmat query){
		cv::Mat queryMat;
		MatrixTools::fmatToMat(query, queryMat);
		return classify(queryMat);
	}

	virtual float classify(vector<float> query){
		cv::Mat queryMat;
		MatrixTools::vectorToMat(query, queryMat);
		return classify(queryMat);
	}
};

