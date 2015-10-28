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

    if(params.size() == 0){
        return;
    }


    cout << "started " << endl;
    _socket.bind(Poco::Net::SocketAddress("0.0.0.0", std::stoi(params["port"])), false);
	_thread.start(*this);
	_ready.wait();
	_stop = false;
	_bufferSize = std::stoi(params["bufferSize"]);
    inBuffer = new char[_bufferSize];

	baseIndex = std::shared_ptr<IIndexer>((IIndexer*)FactoryIndexer::getInstance()->createType(params["baseIndex"]));
	baseIndex->load(params["indexLocation"]);
}

DistributedIndexWrapperServer::~DistributedIndexWrapperServer(){
	_stop = true;
	if(_thread.isRunning())
        _thread.join();
    delete[] inBuffer;
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
    baseIndex->train(featuresTrain,featuresValidationI,featuresValidationQ);
}

void DistributedIndexWrapperServer::indexWithTrainedParams(cv::Mat& features){
    baseIndex->indexWithTrainedParams(features);
}

void DistributedIndexWrapperServer::index(cv::Mat& features){
    baseIndex->index(features);
}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperServer::knnSearchId(cv::Mat& query, int n, double search_limit){
	/*
	vector<float> indices (n);
	vector<float> dists (n);

	for(int i = 0; i < n; i++){
        indices[i] = n-i;
        dists[i] = i + 0.005f;
	}
    return make_pair(indices,dists);
    */
	return baseIndex->knnSearchId(query, n, search_limit);
}

std::pair<vector<float>,vector<float> > DistributedIndexWrapperServer::radiusSearchId(cv::Mat& query, double radius, int n, double search_limit){
	return baseIndex->radiusSearchId(query, radius, n, search_limit);
}

bool DistributedIndexWrapperServer::save(string basePath){
	return baseIndex->save(basePath);
}

bool DistributedIndexWrapperServer::load(string basePath){
	return baseIndex->load(basePath);
}

string DistributedIndexWrapperServer::getName(){
	return type;
}

int DistributedIndexWrapperServer::addToIndexLive(Mat& features){
    return baseIndex->addToIndexLive(features);
}

void DistributedIndexWrapperServer::run(){
    _ready.set();
	Poco::Timespan span(250000);

	cout << "started server at " << _socket.address().host().toString() << ":" << _socket.address().port() << endl;

	while (!_stop){
		if (_socket.poll(span, Poco::Net::Socket::SELECT_READ)){
			try	{
				Poco::Net::SocketAddress sender;
				int n =  _socket.receiveFrom(inBuffer, _bufferSize, sender);
				cout << "server " << _socket.address().host().toString() << ":" << _socket.address().port()  << "  received: " <<  n << " from " << sender.host().toString() << ":" << sender.port() << endl;
                //_socket.sendTo(inputVector, outputVector);

				float* input = reinterpret_cast<float*>(inBuffer);
				vector<float> inputVector, outputVector;
				inputVector.insert( inputVector.begin(), input, input + (n/sizeof(float))) ;
                processQuery(inputVector, outputVector);

                cout << "server " << _socket.address().host().toString() << ":" << _socket.address().port()  << " sent: " << _socket.sendTo(&outputVector[0], outputVector.size()*sizeof(float), sender)  << endl;

			} catch (Poco::Exception& exc){
				std::cerr << "UDPEchoServer: " << exc.displayText() << std::endl;
			}
		}
	}
}

void DistributedIndexWrapperServer::processQuery(vector<float>& input, vector<float>& output){
    int inCode = (int)input[0];
    switch (inCode) {
        case 1: {

            int n = (int)input[1];
            float limit = input[2];

            int rows = (int)input[3];
            int cols = (int)input[4];
            cv::Mat query(rows, cols, CV_32F, &input[5]);
            cv::Mat localQuery;
            query.copyTo(localQuery);

            //for(int i = 0; i < input.size(); i++){
            //        cout << input.at(i) << " ";
            //}
            //cout << endl;

            std::pair<vector<float>,vector<float> > results = knnSearchId(localQuery,n,limit);

            output.push_back(0);
            output.push_back(results.first.size());
            output.insert(output.end(), &results.first[0], &results.first[results.first.size()]);
            output.insert(output.end(), &results.second[0], &results.second[results.second.size()]);
            break;
         } case 2: {

            int rows = (int)input[1];
            int cols = (int)input[2];

            cv::Mat query(rows, cols, CV_32F, &input[3]);

            int result = addToIndexLive(query);

            output.push_back(result);

            break;
        }
  }
}
