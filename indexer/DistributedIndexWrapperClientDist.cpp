#include "DistributedIndexWrapperClientDist.h"

static DistributedIndexWrapperClientDist distIndexerWrapperClientFactoryDist;
static int distIndexerWrapperClientFactoryDistPortCount = 0;

DistributedIndexWrapperClientDist::DistributedIndexWrapperClientDist(){
	FactoryIndexer::getInstance()->registerType("DistributedIndexWrapperClientDist",this);
}

DistributedIndexWrapperClientDist::DistributedIndexWrapperClientDist(string& typeId){
    type = typeId;
}

DistributedIndexWrapperClientDist::DistributedIndexWrapperClientDist(string& typeId, map<string,string>& params){
    type = typeId;
    paramsB = params;

    if(params.size() == 0){
        return;
    }

    clientAddress = Poco::Net::SocketAddress("0.0.0.0", std::stoi(params["port"])+distIndexerWrapperClientFactoryDistPortCount);
    distIndexerWrapperClientFactoryDistPortCount++;
    bufferSize = std::stoi(params["bufferSize"]);
    baseFeatureExtractor = std::shared_ptr<FeatureExtractor>((FeatureExtractor*)FactoryAnalyser::getInstance()->createType(params["extractor"]));

    string serversString = params["servers"];
    vector<string> servers = StringTools::split(serversString,';');
    for (int i = 0; i < servers.size(); i++){
        serverAddresses.push_back(Poco::Net::SocketAddress(servers.at(i)));
    }

    cout << "started client at " << clientAddress.host().toString() << ":" << clientAddress.port() << endl;

}

DistributedIndexWrapperClientDist::~DistributedIndexWrapperClientDist(){

}

void* DistributedIndexWrapperClientDist::createType(string &typeId, map<string,string>& params){
    return new DistributedIndexWrapperClientDist(typeId,params);
}

void* DistributedIndexWrapperClientDist::createType(string &typeId){
	if (typeId == "DistributedIndexWrapperClientDist"){
		return new DistributedIndexWrapperClientDist(typeId);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void DistributedIndexWrapperClientDist::train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ){
    return;
}

void DistributedIndexWrapperClientDist::indexWithTrainedParams(cv::Mat& features){
    return;
}

void DistributedIndexWrapperClientDist::index(cv::Mat& features){
    return;
}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperClientDist::knnSearchId(cv::Mat& query, int n, double search_limit){

    vector<float> indices;
    vector<float> dists;

    vector<Poco::Net::SocketAddress> servers = getRelevantServers(query);



    for(int i = 0; i < servers.size(); i++){
        Poco::Net::SocketAddress server_address = servers.at(i);

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
        cout << output[0] << " " << output[1] << " " << output[2] << endl;

        indices.insert(indices.end(), &output[2], &output[2+n_new]);
        dists.insert(dists.end(), &output[2+n_new], &output[2+2*n_new]);
        
    }
	return make_pair(indices,dists);
}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperClientDist::radiusSearchId(cv::Mat& query, double radius, int n, double search_limit){
	vector<int> indices (n);
	vector<float> dists (n);
	return make_pair(dists,dists);
}

bool DistributedIndexWrapperClientDist::save(string basePath){
	return true;
}

bool DistributedIndexWrapperClientDist::load(string basePath){
	return true;
}

string DistributedIndexWrapperClientDist::getName(){
	return type;
}

int DistributedIndexWrapperClientDist::addToIndexLive(Mat& features){
    vector<Poco::Net::SocketAddress> servers = getRelevantServers(features);

    for(int i = 0; i < servers.size(); i++){
        Poco::Net::SocketAddress server_address = servers.at(i);

        vector<float> input, output;
        input.push_back(2);
        input.push_back(features.rows);
        input.push_back(features.cols);

        float* arrayData = (float*)features.data;

        input.insert(input.end(), &arrayData[0], &arrayData[(int)features.rows*features.cols]);

        sendMessage(input,output,server_address);
    }
    return 1;
}

void DistributedIndexWrapperClientDist::sendMessage(vector<float>& input, vector<float>& output, Poco::Net::SocketAddress& server){
    char* inbuffer = reinterpret_cast<char*>(&input[0]);
    Poco::Net::DatagramSocket dgs(clientAddress);
    cout << "client wants to send: " <<  input.size()*sizeof(float) << endl;

    //for(int i = 0; i < input.size(); i++){
    //        cout << input[i] << " ";
    //}
    //cout << endl;
    cout << clientAddress.host().toString() << " " << clientAddress.port() << endl;
    int sent = dgs.sendTo(&inbuffer[0], input.size()*sizeof(float), server);
    cout << "client " << clientAddress.host().toString() << ":" << clientAddress.port()  << " sent: " << sent << endl;
    char* buffer = new char[bufferSize];

    int floatBufferSize = 0;
    floatBufferSize = dgs.receiveBytes(buffer,bufferSize);
    cout << "client " << clientAddress.host().toString() << ":" << clientAddress.port()  << " received: " << floatBufferSize << endl;
    float* floatBuffer = reinterpret_cast<float*>(buffer);
	output.insert(output.end(), &floatBuffer[0], &floatBuffer[floatBufferSize/sizeof(float)]);
}


vector<Poco::Net::SocketAddress> DistributedIndexWrapperClientDist::getRelevantServers(Mat& query){
    vector<Poco::Net::SocketAddress> results;
    vector<int> resultsIndex;

    cv::Mat sparseRep;
    baseFeatureExtractor->extractFeatures(query,sparseRep);

    int repSize = sparseRep.cols;
    int bucketsPerServer = repSize/(float)serverAddresses.size();
    for(int i = 0; i < serverAddresses.size()-1; i++){
        int count = countNonZero(sparseRep.colRange(i*bucketsPerServer,(i+1)*bucketsPerServer));
        if(count > 0){
            results.push_back(serverAddresses.at(i));
            resultsIndex.push_back(i);
        }
    }
    int count = countNonZero(sparseRep.colRange((serverAddresses.size()-1)*bucketsPerServer,repSize));
    if(count > 0){
        results.push_back(serverAddresses.at((serverAddresses.size()-1)));
        resultsIndex.push_back(serverAddresses.size()-1);
    }

    cout << "relevant servers ";
    for(int i = 0; i < results.size(); i++){
        cout << resultsIndex.at(i) << " ";
    }
    cout << endl;

    return results;

}
