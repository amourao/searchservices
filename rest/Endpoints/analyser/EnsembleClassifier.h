#pragma once

#include <iostream>
#include <sstream>
#include "../../FactoryEndpoint.h"
#include "../../../analyser/FactoryAnalyser.h"
#include "../../IEndpoint.h"
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>
#include <string>
#include <vector>
#include <algorithm>    // std::random_shuffle
#include <vector>       // std::vector
#include <ctime>        // std::time
#include <cstdlib>      // std::rand, std::srand


#include "../../../analyser/nTag/IClassifier.h"

#include "../../../analyser/nVector/FeatureExtractor.h"

#include "../../../analyser/sources/TextFileSource.h"

#include "../../../commons/FileDownloader.h"



class EnsembleClassifier : public IEndpoint
{

public:
	EnsembleClassifier();
	EnsembleClassifier(string type);
	~EnsembleClassifier();

	void* createType(string &typeId);

	void handleRequest(string method, map<string, string> queryStrings, istream&, HTTPServerResponse &resp);

	string getFeatures(map<string, string> parameters);

	void divideByClass(Mat trainData, Mat trainLabels, double numberOfDivisions, int currentDivision,Mat& newTrainData,Mat& newTrainLabels, Mat& testData, Mat& testLabels);
	string crossValidateAll(int numberOfDivisions,string extractorName);
	void readData(string filename, string extractorName);
	string test(const Mat& trainData, const Mat& trainLabels, const Mat& testData, const Mat& testLabels,string extractorName);
	void index(const Mat& trainData);

private:
  string type;
  cv::Mat labels, features;
  cv::flann::Index* flannIndex;
};

