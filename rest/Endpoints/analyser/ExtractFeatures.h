#pragma once

#include <iostream>
#include <sstream>
#include "../../FactoryEndpoint.h"
#include "../../../analyser/FactoryAnalyser.h"
#include "../../IEndpoint.h"
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>


#include "../../../commons/FileDownloader.h"


class ExtractFeatures : public IEndpoint
{

public:
	ExtractFeatures();
	ExtractFeatures(string type);
	~ExtractFeatures();
	
	void* createType(string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream&, HTTPServerResponse &resp);
	
	string getFeatures(map<string, string> parameters);

private:
  string type;
};

