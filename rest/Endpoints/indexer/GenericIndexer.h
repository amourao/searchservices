#pragma once

#include <iostream>
#include <sstream>

#include "../../IEndpoint.h"
#include "../../FactoryEndpoint.h"


#include "../../../analyser/FactoryAnalyser.h"
#include "../../../indexer/FactoryIndexer.h"
#include "../../../indexer/IIndexer.h"

#include "../../../analyser/tools/MatrixTools.h"

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>

#include "../../../commons/FileDownloader.h"

class GenericIndexer: public IEndpoint
{

public:
	GenericIndexer();
	GenericIndexer(std::string type);
	~GenericIndexer();
	
	void* createType(std::string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse &resp);



private:

	string retrieve(map<string, string> queryStrings);
	string create(map<string, string> queryStrings);

	std::string type;
};
