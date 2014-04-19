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

class GetMetadata: public IEndpoint
{

public:
	GetMetadata();
	GetMetadata(std::string type);
	~GetMetadata();

	void* createType(std::string &typeId);

	void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse &resp);



private:

	std::string type;
};
