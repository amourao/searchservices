#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <unordered_set>


#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <Poco/Event.h>

#include <armadillo>

#include <easylogging++.h>

#include "SRStrucs.h"


#include "../../analyser/nVector/FeatureExtractor.h"
#include "../../analyser/FactoryAnalyser.h"
#include "../../analyser/tools/MatrixTools.h"


using namespace std;

class SRMaster {

public:

	SRMaster();
	SRMaster(string& type, map<string,string>& params);
	~SRMaster();

    void train(arma::fmat& featuresTrain,arma::fmat& featuresValidationI,arma::fmat& featuresValidationQ);
	void indexWithTrainedParams(arma::fmat& features);
	void index(arma::fmat& features);

    std::pair<vector<unsigned long>,vector<float> > knnSearchIdLong(arma::fmat& query, int n, float search_limit);

	bool save(string basePath);
	bool load(string basePath);

	int addToIndexLive(arma::fmat& features);

	string getName();

	unsigned long totalQueryTime = 0;
    tp totalQueryTimeStart;

    unsigned long totalAllServerTime = 0;
	tp totalAllServerTimeStart;

    unsigned long totalSortTime = 0;
	tp totalSortTimeStart;

	unsigned long totalCommunicationTime = 0;
	tp totalCommunicationTimeStart;

	unsigned long totalCommunicationSendTime = 0;
	tp totalCommunicationSendTimeStart;

	unsigned long totalCommunicationReceiveTime = 0;
	tp totalCommunicationReceiveTimeStart;

	unsigned long totalSRTime = 0;
	tp totalSRTimeStart;

	unsigned long totalRelServerTime = 0;
	tp totalRelServerTimeStart;

	unsigned long totalMarshallingTime = 0;
	tp totalMarshallingTimeStart;

	unsigned long totalPreMarshallingTime = 0;
	tp totalPreMarshallingTimeStart;

	unsigned long totalNQueries = 0;
	unsigned long missedPackages = 0;

private:



    void sendMessage(vector<QueryStructReq>& query, vector<QueryStructRsp>& output, Poco::Net::SocketAddress& server);

    vector<Poco::Net::SocketAddress> getRelevantServers(arma::fmat& query);

	string type;

	int bufferSize;

	Poco::Net::SocketAddress clientAddress;

	vector<Poco::Net::SocketAddress> serverAddresses;
    std::shared_ptr<FeatureExtractor> baseFeatureExtractor;

    map<string,string> paramsB;


};
