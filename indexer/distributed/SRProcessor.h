#pragma once

#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_set>
#include <limits>

#include <sys/time.h>
#include <sys/resource.h>


#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <Poco/Event.h>

#include "SRStrucs.h"
#include "../../analyser/tools/MatrixTools.h"
#include "../../analyser/tools/oneBillionImporterB.h"


#include <omp.h>


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

	bool loadBilionGIST(string coeffs, string dataPath);
	bool loadCoefsGIST(string coeffs);


	int loadB(string coeffs);

    Poco::Thread _thread;

	unsigned long totalLoadingBucketTime = 0;
	tp totalLoadingBucketTimeStart;

	unsigned long totalLoadingVectorsTime = 0;
	tp totalLoadingVectorsTimeStart;

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



    bool         _stop;

private:

	vector<QueryStructRsp> processQueries(char* input);
	char* processResponses(vector<QueryStructRsp>& responses);
    int getClosestPivot(float coeff, vector<Coefficient>& bucket);


	string type;

    Poco::Net::DatagramSocket _socket;

	Poco::Event  _ready;

	int          _bufferSize;
	long pollInterval;

	uint bucketOffset = 0;
	uint bucketCount = 0;
	ulong offsetDataFile = 0;
	int debugLimitPerBucket = 0;
	int maxIdToLoad = -1;

	bool needsRebuild = false;



};
