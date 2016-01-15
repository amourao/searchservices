#pragma once

#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <Poco/Event.h>

#include "SRStrucs.h"
#include "../../analyser/tools/MatrixTools.h"

#include <armadillo>


using namespace std;

class SRProcessor: public Poco::Runnable {

public:

	SRProcessor();
	SRProcessor(string& type, map<string,string>& params);
	~SRProcessor();

	QueryStructRsp index(QueryStructReq queryS);
	QueryStructRsp knnSearchIdLong(QueryStructReq queryS);

	void rebuild();
	void run();

	bool save(string basePath);
	bool load(string basePath);

private:

	vector<QueryStructRsp> processQueries(char* input);
	char* processResponses(vector<QueryStructRsp>& responses);

	string type;

    Poco::Net::DatagramSocket _socket;
	Poco::Thread _thread;
	Poco::Event  _ready;
	bool         _stop;
	int          _bufferSize;

	uint bucketOffset = 0;

	bool needsRebuild = false;
	
    std::vector<std::vector<Coefficient>> indexData;
    arma::fmat data;
    map<string,string> paramsB;

};
