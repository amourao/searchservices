#include "DistributedIndexWrapperClient.h"

static DistributedIndexWrapperClient distIndexerWrapperClientFactory;

DistributedIndexWrapperClient::DistributedIndexWrapperClient(){
	FactoryIndexer::getInstance()->registerType("DistributedIndexWrapperClient",this);
}

DistributedIndexWrapperClient::DistributedIndexWrapperClient(string& typeId){
    type = typeId;
}

DistributedIndexWrapperClient::DistributedIndexWrapperClient(string& typeId, map<string,string>& params){
    type = typeId;
	paramsB = params;
    client_address = Poco::Net::SocketAddress("localhost", std::stoi(params["port"]));
    bufferSize = 512;
}

DistributedIndexWrapperClient::~DistributedIndexWrapperClient(){

}

void* DistributedIndexWrapperClient::createType(string &typeId, map<string,string>& params){
    return new DistributedIndexWrapperClient(typeId,params);
}

void* DistributedIndexWrapperClient::createType(string &typeId){
	if (typeId == "DistributedIndexWrapperClient"){
		return new DistributedIndexWrapperClient(typeId);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void DistributedIndexWrapperClient::train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ){
    return;
}

void DistributedIndexWrapperClient::indexWithTrainedParams(cv::Mat& features){
    return;
}

void DistributedIndexWrapperClient::index(cv::Mat& features){
    return;
}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperClient::knnSearchId(cv::Mat& query, int n, double search_limit){


    Poco::Net::SocketAddress server_address("localhost", 12345);

    vector<float> input, output;
    input.push_back(1);
    input.push_back(n);
    input.push_back(search_limit);
    input.push_back(query.rows);
    input.push_back(query.cols);

    float* arrayData = (float*)query.data;

    input.insert(input.end(), &arrayData[0], &arrayData[(int)query.rows*query.cols]);

    sendMessage(input,output,server_address);

    int current_i = 2;
    int n_new = output[1];

    vector<float> indices;
    vector<float> dists;

    indices.insert(indices.end(), &output[2], &output[2+n_new]);
    dists.insert(dists.end(), &output[2+n_new], &output[2+2*n_new]);
    /*
    for(int i = 0; i < n_new; i++){
        indices.push_back(output[current_i]);
        current_i++;
    }
    for(int i = 0; i < n_new; i++){
        dists.push_back(output[current_i]);
        current_i++;
    }
    */
	return make_pair(indices,dists);
}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperClient::radiusSearchId(cv::Mat& query, double radius, int n, double search_limit){
	vector<int> indices (n);
	vector<float> dists (n);
	return make_pair(dists,dists);
}

bool DistributedIndexWrapperClient::save(string basePath){
	return true;
}

bool DistributedIndexWrapperClient::load(string basePath){
	return true;
}

string DistributedIndexWrapperClient::getName(){
	return type;
}

int DistributedIndexWrapperClient::addToIndexLive(Mat& features){
    Poco::Net::SocketAddress server_address("localhost", 12345);

    vector<float> input, output;
    input.push_back(2);
    input.push_back(features.rows);
    input.push_back(features.cols);

    float* arrayData = (float*)features.data;

    input.insert(input.end(), &arrayData[0], &arrayData[(int)features.rows*features.cols]);

    sendMessage(input,output,server_address);

    return output[0];
}

void DistributedIndexWrapperClient::sendMessage(vector<float>& input, vector<float>& output, Poco::Net::SocketAddress& server){
    char* inbuffer = reinterpret_cast<char*>(&input[0]);
    Poco::Net::DatagramSocket dgs(client_address);
    dgs.sendTo(&inbuffer[0], input.size()*sizeof(float), server);
    char* buffer = new char[bufferSize];

    int floatBufferSize = 0;
    floatBufferSize = dgs.receiveBytes(buffer,bufferSize);

    float* floatBuffer = reinterpret_cast<float*>(buffer);
	output.insert(output.end(), &floatBuffer[0], &floatBuffer[floatBufferSize]);
}
