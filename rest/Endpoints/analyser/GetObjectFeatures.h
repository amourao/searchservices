#pragma once

#include <iostream>
#include <sstream>
#include "../../FactoryEndpoint.h"
#include "../../../analyser/FactoryAnalyser.h"
#include "../../../analyser/nVector/FeatureExtractor.h"
#include "../../../analyser/tools/MatrixTools.h"
#include "../../IEndpoint.h"

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../../../commons/FileDownloader.h"


//USAGE: http://localhost:9090/getObjectFeatures?url=file:///localstore/novaemotionData/p01/0000.png&detector=FaceDetection&extractor=GaborFace
class GetObjectFeatures : public IEndpoint
{

public:
	GetObjectFeatures();
	GetObjectFeatures(string type);
	~GetObjectFeatures();
	
	void* createType(string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream&, HTTPServerResponse &resp);
	
	string getFeatures(map<string, string> parameters);
	string indexFeatures(map<string, string> parameters);
	

private:
  string type;
};

