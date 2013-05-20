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

#include "SearchResult.h"
#include "CombSearchResult.h"

class MedicalSearchEngine: public IEndpoint
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
	
	
	std::vector<SearchResult> searchSingleImage(string filename, string searchType);
	
	void readTrainingData(string textFile);
	void importIclefData(string textFile);
	void createKnnIndex();

	string resultListToJSON(string query, vector<string> images, string combFunction, std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> sortedList);
	string resultListToHTML(string queryText, vector<string> filenames, string combFunction, std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> sortedList,std::vector<std::vector<SearchResult> > fullSearchResults);
private:


	std::string type;
	std::string extractorName;

	std::map<int,std::string> idToIRI;
	std::map<std::string,std::string> IRItoDOI;
	
	
	
	std::map<std::string,cv::flann::Index*> flannIndexes;
	cv::flann::Index* flannIndex;
	
	cv::Mat features;
	cv::Mat labels; 


	std::map<std::string,MedicalSearchEngine*> instances;  
};

