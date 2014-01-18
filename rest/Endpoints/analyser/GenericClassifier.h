#pragma once

#include <iostream>
#include <sstream>

#include "../../IEndpoint.h"
#include "../../FactoryEndpoint.h"

#include "../../../analyser/FactoryAnalyser.h"
#include "../../../analyser/FactoryClassifier.h"

#include "../../../analyser/nTag/IClassifier.h"

#include "../../../analyser/tools/MatrixTools.h"

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>

#include "../../../commons/FileDownloader.h"

class GenericClassifier: public IEndpoint
{

public:
	GenericClassifier();
	GenericClassifier(std::string type);
	~GenericClassifier();
	
	void* createType(std::string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse &resp);



private:

	string classify(map<string, string> queryStrings);
	string create(map<string, string> queryStrings);

	std::string type;
};
