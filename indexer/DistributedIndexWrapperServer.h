#pragma once

#include <memory>
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

#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Thread.h>
#include <Poco/Event.h>

using namespace std;

class DistributedIndexWrapperServer: public IIndexer, public Poco::Runnable {

public:

	DistributedIndexWrapperServer();
	DistributedIndexWrapperServer(string& type);
	DistributedIndexWrapperServer(string& type, map<string,string>& params);
	~DistributedIndexWrapperServer();

	void* createType(string &typeId);
	void* createType(string &typeId, map<string,string>& params);

    void train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ);
	void indexWithTrainedParams(cv::Mat& features);
	void index(cv::Mat& features);
	int addToIndexLive(cv::Mat& features);

	std::pair<vector<float>,vector<float> > knnSearchId(cv::Mat& query, int n, double search_limit);
	std::pair<vector<unsigned long>,vector<float> > knnSearchIdLong(cv::Mat& query, int n, double search_limit);
	std::pair<vector<float>,vector<float> > radiusSearchId(cv::Mat& query, double radius, int n, double search_limit);

	void train(arma::fmat& featuresTrain,arma::fmat& featuresValidationI,arma::fmat& featuresValidationQ);
	void indexWithTrainedParams(arma::fmat& features);
	void index(arma::fmat& features);
	int addToIndexLive(arma::fmat& features);

	std::pair<vector<float>,vector<float> > knnSearchId(arma::fmat& query, int n, double search_limit);
	std::pair<vector<unsigned long>,vector<float> > knnSearchIdLong(arma::fmat& query, int n, double search_limit);
	std::pair<vector<float>,vector<float> > radiusSearchId(arma::fmat& query, double radius, int n, double search_limit);

	bool save(string basePath);
	bool load(string basePath);

	string getName();

	void run();

private:

    void processQuery(vector<float>& input, vector<float>& output);

	string type;
	std::shared_ptr<IIndexer> baseIndex;

    Poco::Net::DatagramSocket _socket;
	Poco::Thread _thread;
	Poco::Event  _ready;
	bool         _stop;
	int          _bufferSize;

	char* inBuffer;

};
