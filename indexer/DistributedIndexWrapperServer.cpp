#include "DistributedIndexWrapperServer.h"

static DistributedIndexWrapperServer distIndexerWrapperServerFactory;

DistributedIndexWrapperServer::DistributedIndexWrapperServer(){
	FactoryIndexer::getInstance()->registerType("DistributedIndexWrapperServer",this);
}

DistributedIndexWrapperServer::DistributedIndexWrapperServer(string& typeId){
    type = typeId;
}

DistributedIndexWrapperServer::DistributedIndexWrapperServer(string& typeId, map<string,string>& params){
    type = typeId;
	paramsB = params;
}

DistributedIndexWrapperServer::~DistributedIndexWrapperServer(){

}

void* DistributedIndexWrapperServer::createType(string &typeId, map<string,string>& params){
    return new DistributedIndexWrapperServer(typeId,params);
}

void* DistributedIndexWrapperServer::createType(string &typeId){
	if (typeId == "DistributedIndexWrapperServer"){
		return new DistributedIndexWrapperServer(typeId);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void DistributedIndexWrapperServer::train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ){

}

void DistributedIndexWrapperServer::indexWithTrainedParams(cv::Mat& features){

}

void DistributedIndexWrapperServer::index(cv::Mat& features){

}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperServer::knnSearchId(cv::Mat& query, int n, double search_limit){
	vector<int> indices (n);
	vector<float> dists (n);
	return make_pair(dists,dists);
}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperServer::radiusSearchId(cv::Mat& query, double radius, int n, double search_limit){
	vector<int> indices (n);
	vector<float> dists (n);
	return make_pair(dists,dists);
}

bool DistributedIndexWrapperServer::save(string basePath){
	return true;
}

bool DistributedIndexWrapperServer::load(string basePath){
	return true;
}

string DistributedIndexWrapperServer::getName(){
	return type;
}

int DistributedIndexWrapperServer::addToIndexLive(Mat& features){
    return -1;
}
