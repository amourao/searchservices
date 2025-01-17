#pragma once

#include <iostream>
#include <sstream>

#include "../../IEndpoint.h"
#include "../../FactoryEndpoint.h"


#include "../../../analyser/FactoryAnalyser.h"
#include "../../../analyser/nVector/FeatureExtractor.h"
#include "../../../indexer/FactoryIndexer.h"
#include "../../../indexer/IIndexer.h"

#include "../../../analyser/tools/MatrixTools.h"

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>
#include <Poco/StreamCopier.h>

#include "../../../commons/FileDownloader.h"
#include "../../../commons/Timing.h"

class GenericIndexer: public IEndpoint
{

public:
	GenericIndexer();
	GenericIndexer(std::string type);
	~GenericIndexer();

	void* createType(std::string &typeId);

	void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse &resp);


    string retrieve(map<string, string> queryStrings);

    string retrieveExtracted(map<string, string> queryStrings, istream& input);

	string create(map<string, string> queryStrings);
	string addToIndexLive(map<string, string> parameters);
	string saveIndex(map<string, string> parameters);
private:

	std::pair< vector<string>, vector<float> > idToLabels(std::pair< vector<float>, vector<float> > v1);
	IIndexer* getIndexer(string indexerName,string s);

	std::string type;
};
