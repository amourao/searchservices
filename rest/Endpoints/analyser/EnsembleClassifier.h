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
	string crossValidateAll(int numberOfDivisions);
	void readData(string filename, string extractorName);
	string test(Mat& trainData, Mat& trainLabels, Mat& testData, Mat& testLabels);


private:
  string type;
  cv::Mat labels, features;
};

