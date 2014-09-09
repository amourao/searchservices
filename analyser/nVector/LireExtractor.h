#pragma once


#include <iostream>

#include <cstdlib>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <iostream>
#include <string> 
#include <jsoncpp/json/json.h>
					
#include "../FactoryAnalyser.h"
#include "../../commons/StringTools.h"
#include "FeatureExtractor.h"

using namespace Poco::Net;
using namespace Poco;
using namespace std;

class LireExtractor :
	public FeatureExtractor
{
public:

	LireExtractor();
	LireExtractor(string& type);
	LireExtractor(string& type, map<string ,string>& params);
	~LireExtractor();

    void* createType(string &typeId);
    void* createType(string &typeId, map<string ,string>& params);


	void extractFeatures(Mat& src, Mat& dst);
	void extractFeatures(string filename, vector<float>& features);
	void extractFeaturesMulti(string filename, vector<vector<float> >& features);

	int getFeatureVectorSize();

	string getName();
private:

	string type;
	string algorithm;
	string host,endpoint;
};

