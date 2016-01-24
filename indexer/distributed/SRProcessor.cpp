#include "SRProcessor.h"

//static SRProcessor distIndexerWrapperServerFactory;

template <typename T>
SRProcessor<T>::SRProcessor(){
	//FactoryIndexer::getInstance()->registerType("SRProcessor",this);
}

template <typename T>
SRProcessor<T>::SRProcessor(string& typeId, map<string,string>& params){

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

template <typename T>
SRProcessor<T>::~SRProcessor(){
	_stop = true;
	if(_thread.isRunning())
        _thread.join();
}

template <typename T>
QueryStructRsp SRProcessor<T>::index(QueryStructReq queryS){

    arma::fmat featuresT(&queryS.query[0],1,queryS.query.size(),true);
    arma::Mat<T> features = arma::conv_to<arma::Mat<T>>::from(featuresT);

    vector<float> coeffs = queryS.coeffs;
    vector<int> buckets = queryS.buckets;
    vector<uindex> indexes = queryS.indexes;

    uint global_id = indexes[0];
    uint local_id = lidTogid.size();

    lidTogid.push_back(global_id);

    auto compare = [](Coefficient a, Coefficient b){ return a < b; };

    for(uint i = 0; i < coeffs.size(); i++){
        indexData[buckets[i]].push_back(Coefficient(local_id,coeffs[i]));

        auto p = MatrixTools::sortPermutation(indexData[buckets[i]],compare);
        indexData[buckets[i]] = MatrixTools::applyPermutation(indexData[buckets[i]], p, p.size());
    }

    QueryStructRsp result;
    result.operation = 2;
    result.parameters.push_back(0);

    return result;
}

template <typename T>
QueryStructRsp SRProcessor<T>::knnSearchIdLong(QueryStructReq queryS){

    arma::fmat queryT(&queryS.query[0],queryS.query.size(),1,false);
    arma::Mat<T> query = arma::conv_to<arma::Mat<T>>::from(queryT);

    vector<int> buckets = queryS.buckets;

    int n = queryS.parameters[0];
    double search_limit = queryS.parameters[1];

    uint estimatedCandidateSize = 50;

    QueryStructRsp result;

    vector<uindex> indices;
    vector<float> dists;

    indices.reserve(estimatedCandidateSize);
    dists.reserve(estimatedCandidateSize);

    map<uindex, float> computedDists;

    #ifdef MEASURE_TIME
        totalBucketTimeStart = NOW();
    #endif

    #pragma omp parallel for schedule(dynamic)
    for(uint b = 0; b < buckets.size(); b++){
        int bucket = buckets[b]-bucketOffset;
        #ifdef MEASURE_TIME
                totalNBucketsReq++;
        #endif
        if (bucket >= 0 && bucket < indexData.size()){ //is one of this node buckets

            #ifdef MEASURE_TIME
                totalNBucketInsp++;
            #endif

            //cout << type << " " << bucket << " " << bucketOffset << endl;
            uint on = 0;
            if(search_limit <= 1)
                on = indexData[bucket].size()*search_limit;
            else {
                on = std::min((uint)indexData[bucket].size(),(uint)search_limit);
            }

            vector<Coefficient> candidates(indexData[bucket].begin(),indexData[bucket].begin()+on);
            for(uint i = 0; i < candidates.size(); i++){

                #ifdef MEASURE_TIME
                    totalNCandidatesInsp++;
                #endif

                if (computedDists.find(candidates[i].vector_pos) == computedDists.end()){

                    #ifdef MEASURE_TIME
                        totalNCandidatesInspNonDup++;
                    #endif
                    arma::Mat<T> candidate = data.col(candidates[i].vector_pos);
                    arma::Mat<T> diff = candidate - query;
                    float dist = myNorm(diff);
                    uindex lid = candidates[i].vector_pos;
                    uindex gid = lidTogid[lid];
                    #pragma omp critical
                    {
                        indices.push_back(gid);
                        dists.push_back(dist);
                        computedDists[lid] = dist;
                    }
                }
            }
        }
    }

    #ifdef MEASURE_TIME
        totalBucketTime += ELAPSED(totalBucketTimeStart);
        totalSortTimeStart = NOW();
    #endif
    uint newN = min((uint)n,(uint)dists.size());

    auto compare = [](float a, float b){ return a < b; };
    auto p = MatrixTools::sortPermutation(dists,compare);

    result.dists = MatrixTools::applyPermutation(dists, p, newN);
    result.indexes = MatrixTools::applyPermutation(indices, p, newN);
    result.operation = 1;
    result.parameters.push_back(n);
    result.parameters.push_back(newN);
    result.totalByteSize = result.computeTotalByteSize();

    #ifdef MEASURE_TIME
        totalSortTime += ELAPSED(totalSortTimeStart);
    #endif

    return result;
}

template <typename T>
QueryStructRsp SRProcessor<T>::getStatistics(){
    QueryStructRsp result;
    result.operation = 10;

    result.indexes.push_back(totalTime);
    result.indexes.push_back(totalQueryTime);
    result.indexes.push_back(totalCommunicationReceiveTime);
    result.indexes.push_back(totalCommunicationSendTime);
    result.indexes.push_back(totalBucketTime);
    result.indexes.push_back(totalSortTime);
    result.indexes.push_back(totalPreMarshallingTime);
    result.indexes.push_back(totalMarshallingTime);
    result.indexes.push_back(totalNQueries);
    result.indexes.push_back(totalNBucketsReq);
    result.indexes.push_back(totalNBucketInsp);
    result.indexes.push_back(totalNCandidatesInsp);
    result.indexes.push_back(totalNCandidatesInspNonDup);
    result.indexes.push_back(missedPackages);

    totalTime = 0;
    totalQueryTime = 0;
	totalCommunicationReceiveTime = 0;
	totalCommunicationSendTime = 0;
	totalBucketTime = 0;
	totalSortTime = 0;
    totalPreMarshallingTime = 0;
	totalMarshallingTime = 0;

	totalNQueries = 0;
	totalNBucketsReq = 0;
	totalNBucketInsp = 0;
	totalNCandidatesInsp = 0;
	totalNCandidatesInspNonDup = 0;
    missedPackages = 0;

    return result;
}

template <typename T>
void SRProcessor<T>::rebuild(){

}

template <typename T>
void SRProcessor<T>::run(){
    _ready.set();
	Poco::Timespan span(250000);

	cout << "started Processor at " << _socket.address().host().toString() << ":" << _socket.address().port() << endl;

	while (!_stop){
		if (_socket.poll(span, Poco::Net::Socket::SELECT_READ)){

            char* inBuffer = NULL;
			try	{
                #ifdef MEASURE_TIME
                    totalNQueries++;
                    totalTimeStart = NOW();
                    totalCommunicationReceiveTimeStart = NOW();
                #endif
                inBuffer = new char[_bufferSize];
				Poco::Net::SocketAddress sender;

				int n =  _socket.receiveFrom(inBuffer, _bufferSize, sender);

				#ifdef MEASURE_TIME
                    totalCommunicationReceiveTime += ELAPSED(totalCommunicationReceiveTimeStart);
                #endif
                cout << "Processor " << _socket.address().host().toString() << ":" << _socket.address().port()  << "  received: " <<  n << " from " << sender.host().toString() << ":" << sender.port() << endl;
                //_socket.sendTo(inputVector, outputVector);

                //std::ofstream outfile ("proc.bin",std::ofstream::binary);
                //outfile.write (&inBuffer[0],n);
                //outfile.close();

                vector<QueryStructRsp> responses = processQueries(inBuffer);


                #ifdef MEASURE_TIME
                    totalPreMarshallingTimeStart = NOW();
                #endif

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

                #ifdef MEASURE_TIME
                    totalPreMarshallingTime += ELAPSED(totalPreMarshallingTimeStart);
                    totalCommunicationSendTimeStart = NOW();
                #endif

                cout << "Processor " << _socket.address().host().toString() << ":" << _socket.address().port()  << " sent: " << _socket.sendTo(&outbuffer[0], totalSize, sender) << endl;

                #ifdef MEASURE_TIME
                    totalCommunicationSendTime += ELAPSED(totalCommunicationSendTimeStart);
                    totalTime += ELAPSED(totalTimeStart);
                #endif

			} catch (Poco::Exception& exc){
                #ifdef MEASURE_TIME
                    missedPackages++;
                #endif
                cerr << "UDPEchoServer: " << exc.displayText() << endl;
			}
            if(inBuffer != NULL)
                delete[] inBuffer;

		}
	}
}

template <typename T>
vector<QueryStructRsp> SRProcessor<T>::processQueries(char* input){
    uint numOps = (uint)input[0];
    uint accumBytes = 1;
    vector<QueryStructRsp> result;

    for (uint i = 0; i < numOps; i++){
        #ifdef MEASURE_TIME
            totalMarshallingTimeStart = NOW();
        #endif
        char* newInput = &input[accumBytes];

        QueryStructReq query;
        query.toQueryStructReq(newInput);
        accumBytes += query.totalByteSize;
        char op = query.operation;
        QueryStructRsp resultTmp;
        #ifdef MEASURE_TIME
            totalMarshallingTime += ELAPSED(totalMarshallingTimeStart);
        #endif
        if (op == 1){
            #ifdef MEASURE_TIME
                totalQueryTimeStart = NOW();
            #endif
            resultTmp = knnSearchIdLong(query);
            #ifdef MEASURE_TIME
                totalQueryTime += ELAPSED(totalQueryTimeStart);
            #endif
        } else if (op == 2){
            resultTmp = index(query);
            needsRebuild = true;
        } else if (op == 3){
            resultTmp = index(query);
            rebuild();
        } else if (op == 10){
            resultTmp = getStatistics();
        } else {
            resultTmp.operation = query.operation;
            resultTmp.parameters.push_back(-1);
        }
        resultTmp.totalByteSize = resultTmp.computeTotalByteSize();
        result.push_back(resultTmp);
    }
    return result;
}

template <typename T>
char* SRProcessor<T>::processResponses(vector<QueryStructRsp>& responses){
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

template <typename T>
bool SRProcessor<T>::save(string basePath){
    uint numBuckets = indexData.size();
    uint totalSize = sizeof(uint)+sizeof(uint)*numBuckets ;
    uint sizeOfCoeff = sizeof(uindex)*1 + sizeof(float);
    uint curr = 0;
    for(uint i = 0; i < indexData.size(); i++){
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

            memcpy(&dataToSave[curr],&c.vector_pos,sizeof(uindex));
            curr += sizeof(uindex);

            memcpy(&dataToSave[curr],&c.value,sizeof(float));
            curr += sizeof(float);

        }
    }
    std::ofstream outfile (basePath + "_coeffs.bin",std::ofstream::binary);
    outfile.write (&dataToSave[0],curr);
    outfile.close();

    std::ofstream outfile2 (basePath + "_lidtogid.bin",std::ofstream::binary);
    outfile2.write ((char*)&lidTogid[0],sizeof(uint)*lidTogid.size());
    outfile2.close();

    delete[] dataToSave;

    data.save(basePath + "_features.bin");

    return 0;
}

template <typename T>
bool SRProcessor<T>::loadAll(string basePath){

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
            uindex vector_pos = *reinterpret_cast<uindex*>(&buffer[curr]);
            curr += sizeof(uindex);

            uindex value = *reinterpret_cast<float*>(&buffer[curr]);
            curr += sizeof(float);

            indexData[i].push_back(Coefficient(vector_pos,value));
        }
    }

    delete[] buffer;

    data.load(basePath + "_features.bin");

    return true;
}

template <typename T>
bool SRProcessor<T>::loadSingle(string basePath){

    std::ifstream file2(basePath + "_lidtogid.bin", std::ios::binary | std::ios::ate);
    std::streamsize size2 = file2.tellg();
    file2.seekg(0, std::ios::beg);

    lidTogid = vector<uint>(size2/sizeof(uint));

    if (!file2.read((char*)&lidTogid[0], size2)){
        return false;
    }
    file2.close();


    std::ifstream file(basePath + "_coeffs.bin", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];

    if (!file.read(buffer, size)){
        return false;
    }
    file.close();

    uint curr = 0;
    //uint nBuckets = *reinterpret_cast<uint*>(&buffer[curr]);
    curr += sizeof(uint);

    //bucketOffset = std::stoi(params["bucketOffset"]);
	//bucketCount = std::stoi(params["bucketCount"]);

    indexData = std::vector<std::vector<Coefficient>>(bucketCount);

    for(long i = 0; i < bucketOffset; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint) + (sizeof(uindex)*1+sizeof(float))*co;
        //cout << co << " ";
    }

    //cout << endl << nBuckets << " " << bucketOffset << " " << bucketCount << " " << curr << endl;
    for(long i = 0; i < bucketCount; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint);

        for(uint j = 0; j < co; j++){
            uindex vector_pos = *reinterpret_cast<uindex*>(&buffer[curr]);
            curr += sizeof(uindex);

            float value = *reinterpret_cast<float*>(&buffer[curr]);
            curr += sizeof(float);

            indexData[i].push_back(Coefficient(vector_pos,value));
        }
    }

    delete[] buffer;

    data.load(basePath + "_features.bin");

    return true;
}


template <typename T>
bool SRProcessor<T>::loadBilion(string coeffs, string dataPath){

    lidTogid = vector<uindex>();
    map<uindex,uindex> lidTogidMap;

    std::ifstream file(coeffs, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];

    if (!file.read(buffer, size)){
        return false;
    }
    file.close();

    uint curr = 0;
    //uint nBuckets = *reinterpret_cast<uint*>(&buffer[curr]);
    curr += sizeof(uint);

    //bucketOffset = std::stoi(params["bucketOffset"]);
	//bucketCount = std::stoi(params["bucketCount"]);

    indexData = std::vector<std::vector<Coefficient>>(bucketCount);

    for(long i = 0; i < bucketOffset; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint) + (sizeof(uindex)+sizeof(float))*co;
        //cout << co << " ";
    }

    //cout << endl << nBuckets << " " << bucketOffset << " " << bucketCount << " " << curr << endl;
    for(long i = 0; i < bucketCount; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint);

        for(uint j = 0; j < co; j++){
            uindex gid = *reinterpret_cast<uindex*>(&buffer[curr]);
            curr += sizeof(uindex);

            auto search = lidTogidMap.find(gid);
            uindex lid = lidTogid.size();
            if(search != lidTogidMap.end())
                lid = search->second;
            else {
                lidTogidMap[gid] = lid;
                lidTogid.push_back(gid);
            }

            float value = *reinterpret_cast<float*>(&buffer[curr]);
            curr += sizeof(float);

            indexData[i].push_back(Coefficient(lid,value));
        }
    }

    delete[] buffer;

    arma::uvec ttt(lidTogid.size());

    for(int i = 0; i < lidTogid.size(); i++){
        ttt(i) = lidTogid[i];
    }

    oneBillionImporterB ob;
    ob.readBin(dataPath,data,lidTogid);

    return true;
}



template <typename T>
bool SRProcessor<T>::load(string basePath){

    lidTogid = vector<uindex>();
    map<uindex,uindex> lidTogidMap;

    std::ifstream file(basePath + "_coeffs.bin", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];

    if (!file.read(buffer, size)){
        return false;
    }
    file.close();

    uint curr = 0;
    //uint nBuckets = *reinterpret_cast<uint*>(&buffer[curr]);
    curr += sizeof(uint);

    //bucketOffset = std::stoi(params["bucketOffset"]);
	//bucketCount = std::stoi(params["bucketCount"]);

    indexData = std::vector<std::vector<Coefficient>>(bucketCount);

    for(long i = 0; i < bucketOffset; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint) + (sizeof(uindex)+sizeof(float))*co;
        //cout << co << " ";
    }

    //cout << endl << nBuckets << " " << bucketOffset << " " << bucketCount << " " << curr << endl;
    for(long i = 0; i < bucketCount; i++){
        uint co = *reinterpret_cast<uint*>(&buffer[curr]);
        curr += sizeof(uint);

        for(uint j = 0; j < co; j++){
            uindex gid = *reinterpret_cast<uindex*>(&buffer[curr]);
            curr += sizeof(uindex);

            auto search = lidTogidMap.find(gid);
            uindex lid = lidTogid.size();
            if(search != lidTogidMap.end())
                lid = search->second;
            else {
                lidTogidMap[gid] = lid;
                lidTogid.push_back(gid);
            }

            float value = *reinterpret_cast<float*>(&buffer[curr]);
            curr += sizeof(float);

            indexData[i].push_back(Coefficient(lid,value));
        }
    }

    delete[] buffer;

    arma::uvec ttt(lidTogid.size());

    for(int i = 0; i < lidTogid.size(); i++){
        ttt(i) = lidTogid[i];
    }

    data.load(basePath + "_features.bin");
    data = data.cols(ttt);

    return true;
}


template class SRProcessor<int>;
template class SRProcessor<float>;
template class SRProcessor<uchar>;
