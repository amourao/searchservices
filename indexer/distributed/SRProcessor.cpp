#include "SRProcessor.h"

//static SRProcessor distIndexerWrapperServerFactory;

SRProcessor::SRProcessor(){
	//FactoryIndexer::getInstance()->registerType("SRProcessor",this);
}

SRProcessor::SRProcessor(string& typeId, map<string,string>& params){

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

}

SRProcessor::~SRProcessor(){
	_stop = true;
	if(_thread.isRunning())
        _thread.join();
}

QueryStructRsp SRProcessor::index(QueryStructReq queryS){

    arma::fmat features(&queryS.query[0],1,queryS.query.size(),true);
    vector<float> coeffs = queryS.coeffs;
    vector<int> buckets = queryS.buckets;
    vector<unsigned long> indexes = queryS.indexes;

    auto compare = [](float a, float b){ return a < b; };
    auto p = MatrixTools::sortPermutation(coeffs,compare);
    coeffs = MatrixTools::applyPermutation(coeffs, p, p.size());
    indexes = MatrixTools::applyPermutation(indexes, p, p.size());

    for(uint i = 0; i < coeffs.size(); i++){
        indexData[buckets[i]].push_back(Coefficient(data.n_cols,indexes[i],coeffs[i]));
    }
    data = features;

    QueryStructRsp result;
    result.operation = 2;
    result.parameters.push_back(0);

    return result;
}

QueryStructRsp SRProcessor::knnSearchIdLong(QueryStructReq queryS){

    arma::fmat query(&queryS.query[0],1,queryS.query.size(),false);
    vector<int> buckets = queryS.buckets;

    int n = queryS.parameters[0];
    double search_limit = queryS.parameters[1];

    uint estimatedCandidateSize = indexData[0].size()*indexData.size()/3*search_limit;

    QueryStructRsp result;


    vector<unsigned long> indices;
    vector<float> dists;

    indices.reserve(estimatedCandidateSize);
    dists.reserve(estimatedCandidateSize);

    map<unsigned long, float> computedDists;

    for(uint b = 0; b < buckets.size(); b++){
        uint bucket = buckets[b]-bucketOffset;
        if (bucket < indexData.size()){ //is one of this node buckets
            uint on = indexData[b].size()*search_limit;
            vector<Coefficient> candidates(indexData[b].begin(),indexData[b].begin()+on);
            for(uint i = 0; i < candidates.size(); i++){
                if (computedDists.find(candidates[i].original_id) != computedDists.end()){
                    float dist = norm(data.col(candidates[i].vector_pos) - query, 2);
                    long index = candidates[i].original_id;

                    indices.push_back(index);
                    dists.push_back(dist);

                    computedDists[index] = dist;
                }
            }
        }
    }
    uint newN = min((uint)n,(uint)dists.size());

    auto compare = [](float a, float b){ return a < b; };
    auto p = MatrixTools::sortPermutation(dists,compare);

    result.dists = MatrixTools::applyPermutation(dists, p, newN);
    result.indexes = MatrixTools::applyPermutation(indices, p, newN);
    result.operation = 1;
    result.parameters.push_back(n);
    result.parameters.push_back(newN);

    return result;
}

void SRProcessor::rebuild(){

}

void SRProcessor::run(){
    _ready.set();
	Poco::Timespan span(250000);

	cout << "started server at " << _socket.address().host().toString() << ":" << _socket.address().port() << endl;

	while (!_stop){
		if (_socket.poll(span, Poco::Net::Socket::SELECT_READ)){

            char* inBuffer = NULL;
			try	{
                inBuffer = new char[_bufferSize];
				Poco::Net::SocketAddress sender;
				int n =  _socket.receiveFrom(inBuffer, _bufferSize, sender);
				cout << "server " << _socket.address().host().toString() << ":" << _socket.address().port()  << "  received: " <<  n << " from " << sender.host().toString() << ":" << sender.port() << endl;
                //_socket.sendTo(inputVector, outputVector);
                vector<QueryStructRsp> responses = processQueries(inBuffer);

                char numOps = (char)responses.size();
                uint totalSize = 1;
                for (uint i = 0; i < responses.size(); i++){
                    totalSize+=responses[i].totalByteSize;
                }
                char* outbuffer = new char[totalSize];
                outbuffer[0] = numOps;

                uint curByte = 1;
                for (uint i = 0; i < responses.size(); i++){
                    char* tmp = responses[i].toBytes();
                    memcpy(&outbuffer[curByte],&tmp[0],responses[i].totalByteSize);
                    curByte+=responses[i].totalByteSize;
                }

                cout << "server " << _socket.address().host().toString() << ":" << _socket.address().port()  << " sent: " << _socket.sendTo(&outbuffer[0], totalSize, sender)  << endl;

			} catch (Poco::Exception& exc){
				std::cerr << "UDPEchoServer: " << exc.displayText() << std::endl;
			}
            if(inBuffer != NULL)
                delete[] inBuffer;

		}
	}
}

vector<QueryStructRsp> SRProcessor::processQueries(char* input){
    uint numOps = (uint)input[0];
    uint accumBytes = 1;
    vector<QueryStructRsp> result;

    for (uint i = 0; i < numOps; i++){
        char* newInput = &input[accumBytes];

        QueryStructReq query;
        query.toQueryStructReq(newInput);

        char op = query.operation;
        accumBytes += query.totalByteSize;

        QueryStructRsp resultTmp;
        if (op == 1){
            resultTmp = knnSearchIdLong(query);
        } else if (op == 2){
            resultTmp = index(query);
            needsRebuild = true;
        } else if (op == 3){
            resultTmp = index(query);
            rebuild();
        }else {
            resultTmp.operation = query.operation;
            resultTmp.parameters.push_back(-1);
        }
        result.push_back(resultTmp);
    }
    return result;
}

char* SRProcessor::processResponses(vector<QueryStructRsp>& responses){
    char numOps = (char)responses.size();
    uint totalSize = 1;
    for (uint i = 0; i < responses.size(); i++){
        totalSize+=responses[i].totalByteSize;
    }
    char* response = new char[totalSize];
    response[0] = numOps;

    uint curByte = 1;
    for (uint i = 0; i < responses.size(); i++){
        char* tmp = responses[i].toBytes();
        memcpy(&response[curByte],&tmp[0],responses[i].totalByteSize);
        curByte+=responses[i].totalByteSize;
    }

    return response;
}


bool SRProcessor::save(string basePath){
    return true;
}

bool SRProcessor::load(string basePath){

    return true;
}
