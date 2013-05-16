#pragma once

#include <iostream>
#include <sstream>
#include "../../FactoryEndpoint.h"
#include "../../../analyser/FactoryAnalyser.h"
#include "../../../analyser/tools/MatrixTools.h"
#include "../../IEndpoint.h"
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>


#include "../../../commons/FileDownloader.h"



class MedicalSearchEngine : public IEndpoint
{

public:
	MedicalSearchEngine();
	MedicalSearchEngine(std::string type);
	~MedicalSearchEngine();
	
	void* createType(std::string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream&, HTTPServerResponse &resp);
	std::string medicalSearch(map<string, string> queryStrings);
	std::vector<float>* getFeatures(std::string filename);
	
	void trainIndex(std::string trainData);
	
	void indexFeature(std::vector<float> features, int id);
	void importIclefData(string textFile);
	void createKnnIndex(string baseFile);

private:


	std::string type;
	std::string extractorName;

	std::map<int,std::string> idToIRI;
	std::map<std::string,std::string> IRItoDOI;
	
	cv::flann::Index* flannIndex;
	
	cv::Mat features;
	cv::Mat labels; 

  
};

