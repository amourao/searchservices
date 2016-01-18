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

    bucketOffset = std::stoi(params["bucketOffset"]);
	bucketCount = std::stoi(params["bucketCount"]);
    _bufferSize = std::stoi(params["bufferSize"]);

    _socket.bind(Poco::Net::SocketAddress("0.0.0.0", std::stoi(params["port"])), false);
	_thread.start(*this);
	_ready.wait();
	_stop = false;

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

    arma::fmat query(&queryS.query[0],queryS.query.size(),1,false);
    vector<int> buckets = queryS.buckets;

    int n = queryS.parameters[0];
    double search_limit = queryS.parameters[1];

    uint estimatedCandidateSize = 50;


    QueryStructRsp result;

    vector<unsigned long> indices;
    vector<float> dists;

    indices.reserve(estimatedCandidateSize);
    dists.reserve(estimatedCandidateSize);

    map<unsigned long, float> computedDists;

    //#pragma omp parallel for schedule(dynamic)
    for(uint b = 0; b < buckets.size(); b++){
        int bucket = buckets[b]-bucketOffset;
        if (bucket >= 0 && bucket < indexData.size()){ //is one of this node buckets
            //cout << type << " " << bucket << " " << bucketOffset << endl;
            uint on = indexData[bucket].size()*search_limit;

            vector<Coefficient> candidates(indexData[bucket].begin(),indexData[bucket].begin()+on);
            for(uint i = 0; i < candidates.size(); i++){
                if (computedDists.find(candidates[i].original_id) == computedDists.end()){
                    float dist = norm(data.col(candidates[i].vector_pos) - query, 2);
                    long index = candidates[i].original_id;
                    //#pragma omp critical
                    //{
                        indices.push_back(index);
                        dists.push_back(dist);
                        computedDists[index] = dist;
                    //}

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
    result.totalByteSize = result.computeTotalByteSize();

    return result;
}

void SRProcessor::rebuild(){

}

void SRProcessor::run(){
    _ready.set();
	Poco::Timespan span(250000);

	LOG(INFO) << "started Processor at " << _socket.address().host().toString() << ":" << _socket.address().port();

	while (!_stop){
		if (_socket.poll(span, Poco::Net::Socket::SELECT_READ)){

            char* inBuffer = NULL;
			try	{
                inBuffer = new char[_bufferSize];
				Poco::Net::SocketAddress sender;
				int n =  _socket.receiveFrom(inBuffer, _bufferSize, sender);
                LOG(INFO) << "Processor " << _socket.address().host().toString() << ":" << _socket.address().port()  << "  received: " <<  n << " from " << sender.host().toString() << ":" << sender.port();
                //_socket.sendTo(inputVector, outputVector);

                //std::ofstream outfile ("proc.bin",std::ofstream::binary);
                //outfile.write (&inBuffer[0],n);
                //outfile.close();

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

                LOG(INFO) << "Processor " << _socket.address().host().toString() << ":" << _socket.address().port()  << " sent: " << _socket.sendTo(&outbuffer[0], totalSize, sender) ;

			} catch (Poco::Exception& exc){
                CLOG(ERROR,"Processor") << "UDPEchoServer: " << exc.displayText();
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
        accumBytes += query.totalByteSize;
        char op = query.operation;
        QueryStructRsp resultTmp;
        if (op == 1){
            resultTmp = knnSearchIdLong(query);
        } else if (op == 2){
            resultTmp = index(query);
            needsRebuild = true;
        } else if (op == 3){
            resultTmp = index(query);
            rebuild();
        } else {
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
    uint numBuckets = indexData.size();
    uint totalSize = sizeof(uint)+sizeof(uint)*numBuckets ;
    uint sizeOfCoeff = sizeof(unsigned long)*2 + sizeof(float);
    uint curr = 0;
    for(long i = 0; i < indexData.size(); i++){
        totalSize+=sizeOfCoeff*indexData[i].size();
    }

    char* dataToSave = new char[totalSize];
    memcpy(&dataToSave[curr],&numBuckets,sizeof(uint));
    curr += sizeof(uint);

    for(uint i = 0; i < indexData.size(); i++){
        uint bSize = indexData[i].size();
        memcpy(&dataToSave[curr],&bSize,sizeof(uint));
        curr += sizeof(uint);

        for(uint j = 0; j < indexData[i].size(); j++){
            Coefficient c = indexData[i][j];
            //cout << c.vector_pos << endl;
            memcpy(&dataToSave[curr],&c.vector_pos,sizeof(unsigned long));
            curr += sizeof(unsigned long);

            memcpy(&dataToSave[curr],&c.original_id,sizeof(unsigned long));
            curr += sizeof(unsigned long);

            memcpy(&dataToSave[curr],&c.value,sizeof(float));
            curr += sizeof(float);

        }
    }
    std::ofstream outfile (basePath + "_coeffs.bin",std::ofstream::binary);
    outfile.write (&dataToSave[0],curr);
    outfile.close();

    delete[] dataToSave;

    data.save(basePath + "_features.bin");
}

bool SRProcessor::loadAll(string basePath){

    std::ifstream file(basePath + "_coeffs.bin", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];

    if (!file.read(buffer, size)){
        return false;
    }

    uint curr = 0;
    uint nBuckets = *reinterpret_cast<uint*>(&buffer[curr]);
    curr += sizeof(uint);

    indexData = std::vector<std::vector<Coefficient>>(nBuckets);

    for(long i = 0; i < nBuckets; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint);

        for(uint j = 0; j < co; j++){
            unsigned long vector_pos = *reinterpret_cast<unsigned long*>(&buffer[curr]);
            curr += sizeof(unsigned long);

            unsigned long original_id = *reinterpret_cast<unsigned long*>(&buffer[curr]);
            curr += sizeof(unsigned long);

            unsigned long value = *reinterpret_cast<float*>(&buffer[curr]);
            curr += sizeof(float);

            indexData[i].push_back(Coefficient(vector_pos,original_id,value));
        }
    }

    delete[] buffer;

    data.load(basePath + "_features.bin");

    return true;
}

bool SRProcessor::load(string basePath){

    std::ifstream file(basePath + "_coeffs.bin", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];

    if (!file.read(buffer, size)){
        return false;
    }

    uint curr = 0;
    uint nBuckets = *reinterpret_cast<uint*>(&buffer[curr]);
    curr += sizeof(uint);

    //bucketOffset = std::stoi(params["bucketOffset"]);
	//bucketCount = std::stoi(params["bucketCount"]);

    indexData = std::vector<std::vector<Coefficient>>(bucketCount);

    for(long i = 0; i < bucketOffset; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint) + (sizeof(unsigned long)*2+sizeof(float))*co;
    }
    for(long i = 0; i < bucketCount; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint);

        for(uint j = 0; j < co; j++){
            unsigned long vector_pos = *reinterpret_cast<unsigned long*>(&buffer[curr]);
            curr += sizeof(unsigned long);

            unsigned long original_id = *reinterpret_cast<unsigned long*>(&buffer[curr]);
            curr += sizeof(unsigned long);

            unsigned long value = *reinterpret_cast<float*>(&buffer[curr]);
            curr += sizeof(float);

            indexData[i].push_back(Coefficient(vector_pos,original_id,value));
        }
    }

    delete[] buffer;

    data.load(basePath + "_features.bin");

    return true;
}
