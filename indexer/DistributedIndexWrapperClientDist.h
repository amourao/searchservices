#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>

#include "IIndexer.h"
#include "FactoryIndexer.h"

#include "../commons/FileDownloader.h"
#include "../analyser/nVector/FeatureExtractor.h"
#include "../analyser/FactoryAnalyser.h"

#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <Poco/Event.h>

using namespace std;

class DistributedIndexWrapperClientDist: public IIndexer {

public:

	DistributedIndexWrapperClientDist();
	DistributedIndexWrapperClientDist(string& type);
	DistributedIndexWrapperClientDist(string& type, map<string,string>& params);
	~DistributedIndexWrapperClientDist();

	void* createType(string &typeId);
	void* createType(string &typeId, map<string,string>& params);

    void train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ);
	void indexWithTrainedParams(cv::Mat& features);
	void index(cv::Mat& features);

	std::pair<vector<float>,vector<float> > knnSearchId(cv::Mat& name, int n, double search_limit);
	std::pair<vector<float>,vector<float> > radiusSearchId(cv::Mat& name, double radius, int n, double search_limit);

	bool save(string basePath);
	bool load(string basePath);

	int addToIndexLive(cv::Mat& features);

	string getName();

private:

    void sendMessage(vector<float>& input, vector<float>& output, Poco::Net::SocketAddress& server);

    vector<Poco::Net::SocketAddress> getRelevantServers(cv::Mat& query);

	string type;

	int bufferSize;

	Poco::Net::SocketAddress clientAddress;

	vector<Poco::Net::SocketAddress> serverAddresses;
    std::shared_ptr<FeatureExtractor> baseFeatureExtractor;


};
