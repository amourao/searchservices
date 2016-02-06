#pragma once

#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_set>

#include <sys/time.h>
#include <sys/resource.h>


#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <Poco/Event.h>

#include <easylogging++.h>

#include "SRStrucs.h"
#include "../../analyser/tools/MatrixTools.h"
#include "../../analyser/tools/oneBillionImporterB.h"


#include <armadillo>

#include <string>
#include <iostream>
#include <cstdio>
#include <memory>

using namespace std;

template <typename T>
class SRProcessor: public Poco::Runnable {

public:

	SRProcessor();
	SRProcessor(string& type, map<string,string>& params);
	~SRProcessor();

	QueryStructRsp index(QueryStructReq& queryS);
	QueryStructRsp knnSearchIdLong(QueryStructReq& queryS);
	QueryStructRsp getStatistics();

	void rebuild();
	void run();

	bool save(string basePath);
	bool load(string basePath);
	bool loadSingle(string basePath);
	bool loadAll(string basePath);
	bool loadBilion(string coeffs, string dataPath);
	bool loadBilionMultiFile(string coeffs, string dataPath);

	int loadB(string coeffs);

    Poco::Thread _thread;

    unsigned long totalTime = 0;
    tp totalTimeStart;

    unsigned long totalQueryTime = 0;
    tp totalQueryTimeStart;

	unsigned long totalCommunicationReceiveTime = 0;
	tp totalCommunicationReceiveTimeStart;

	unsigned long totalCommunicationSendTime = 0;
	tp totalCommunicationSendTimeStart;

	unsigned long totalBucketTime = 0;
	tp totalBucketTimeStart;

	unsigned long totalSortTime = 0;
	tp totalSortTimeStart;

    unsigned long totalPreMarshallingTime = 0;
	tp totalPreMarshallingTimeStart;

	unsigned long totalMarshallingTime = 0;
	tp totalMarshallingTimeStart;

	unsigned long totalNQueries = 0;
	unsigned long totalNBucketsReq = 0;
	unsigned long totalNBucketInsp = 0;
	unsigned long totalNCandidatesInsp = 0;
	unsigned long totalNCandidatesInspNonDup = 0;
    unsigned long missedPackages = 0;


    std::vector<std::vector<Coefficient>> indexData;
    arma::Mat<T> data;
    map<string,string> paramsB;
    vector<uindex> lidTogid;
private:

	vector<QueryStructRsp> processQueries(char* input);
	char* processResponses(vector<QueryStructRsp>& responses);

	string type;

    Poco::Net::DatagramSocket _socket;

	Poco::Event  _ready;
	bool         _stop;
	int          _bufferSize;
	long pollInterval;

	uint bucketOffset = 0;
	uint bucketCount = 0;
	int debugLimitPerBucket = 0;
	int maxIdToLoad = -1;

	bool needsRebuild = false;



};
