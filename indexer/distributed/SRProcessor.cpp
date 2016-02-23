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
    pollInterval = std::stol(params["pollInterval"]);
    debugLimitPerBucket = std::stoi(params["debugLimitPerBucket"]);
    maxIdToLoad = std::stoi(params["maxIdToLoad"]);

    if(params.count("offsetDataFile") > 0)
        offsetDataFile = std::stol(params["offsetDataFile"]);

    if(params.count("startFromPivot") > 0)
        startFromPivot = true;

    if(params.count("dontFinalSort") > 0)
        doFinalSort = false;

    if(params.count("doFinalSortCoeff") > 0)
        doFinalSortCoeff = true;

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
QueryStructRsp SRProcessor<T>::index(QueryStructReq& queryS){

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
    result.parameters.push_back(global_id);

    return result;
}

template <typename T>
QueryStructRsp SRProcessor<T>::knnSearchIdLong(QueryStructReq& queryS){

    arma::fmat queryT(&queryS.query[0],queryS.query.size(),1,false);
    arma::Mat<T> query = arma::conv_to<arma::Mat<T>>::from(queryT);

    vector<int> buckets = queryS.buckets;

    int n = queryS.parameters[0];
    double search_limit = queryS.parameters[1];
    float nQuery = queryS.parameters[2];

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

    //#pragma omp parallel for schedule(dynamic)
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
            vector<Coefficient> candidates;
            if (!startFromPivot){
                candidates = vector<Coefficient>(indexData[bucket].begin(),indexData[bucket].begin()+on);
            } else {
                if(on == indexData[bucket].size()){ //search full bucket anyway

                    candidates = vector<Coefficient>(indexData[bucket].begin(),indexData[bucket].begin()+on);

               } else if (indexData[bucket].size() > 0){
                    int b = 0;
                    while(queryS.buckets[b] != bucket)
                        b++;
                    float queryBucketCoeff = queryS.coeffs[b];
                    uint closestPivotIndex = getClosestPivot(queryBucketCoeff,indexData[bucket]);
                    uint overflow = 0;
                    int start = closestPivotIndex-on/2-1;
                    if(start < 0){
                        overflow = -start;
                        start = 0;
                    }
                    uint end = closestPivotIndex+overflow+on/2;
                    if(end >= indexData[bucket].size()){
                        end = indexData[bucket].size()-1;
                    }
                    //  cout << endl << "b: " << indexData[bucket].size() << closestPivotIndex << " " << indexData[bucket][closestPivotIndex].vector_pos << " " << closestPivotIndex <<  " " << start << " " << end << " " << indexData[bucket].size() << endl << endl;
                    for(uint bi = start; bi < end; bi++){
                        candidates.push_back(indexData[bucket][bi]);
                        //cout << bi << " " << indexData[bucket][bi].vector_pos << endl;
                    }
                    //candidates = vector<Coefficient>(indexData[bucket].begin()+start,indexData[bucket].begin()+end);
                }
            }


            for(uint i = 0; i < candidates.size(); i++){

                #ifdef MEASURE_TIME
                    totalNCandidatesInsp++;
                #endif

                if (computedDists.find(candidates[i].vector_pos) == computedDists.end()){

                    #ifdef MEASURE_TIME
                        totalNCandidatesInspNonDup++;
                    #endif
                    arma::Mat<T> candidate = data.col(candidates[i].vector_pos);
                    //normalizeColumns(candidate);
                    //normalizeColumns(query);
                    float dist = myNorm(candidate,query);
                    uindex lid = candidates[i].vector_pos;
                    uindex gid = lidTogid[lid];
                    //cout << "c: " << lid << " " << gid << " " <<  (float)candidate.at(2,0) << " " << (float)query.at(2,0) <<  " " << dist << endl;
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
    result.parameters.push_back(nQuery);
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
	Poco::Timespan span(pollInterval);

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
                //cout << "Processor " << _socket.address().host().toString() << ":" << _socket.address().port()  << "  received: " <<  n << " from " << sender.host().toString() << ":" << sender.port() << endl;
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

                n = _socket.sendTo(&outbuffer[0], totalSize, sender);
                //cout << "Processor " << _socket.address().host().toString() << ":" << _socket.address().port()  << " sent: " << n << endl;

                #ifdef MEASURE_TIME
                    totalCommunicationSendTime += ELAPSED(totalCommunicationSendTimeStart);
                    totalTime += ELAPSED(totalTimeStart);
                #endif
                if(responses.size() > 0){
                    if(responses[0].operation == 1){
                        /*
                        cout << "R;" << responses[0].parameters[2] << ";";
                        for(uint j = 0; j < responses[0].indexes.size(); j++){
                            cout << responses[0].indexes[j] << "," << responses[0].dists[j] << ";";
                        }
                        cout << endl;
                        */
                    } else if(responses[0].operation == 10){
                        cout << "Sending statistics" << endl;
                    }

                }

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

    map<uindex,uindex> lidTogidMap;

    FILE * file = fopen(coeffs.c_str(), "rb" );

    //fseek(file, 0L, SEEK_END);
    //unsigned long long sz = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char* buffer = new char[sizeof(uint)];
    size_t result;
    cout << "Loading coefficients" << endl;

    unsigned long long curr = 0;
    result = fread (buffer,1,sizeof(uint),file);
    //uint nBuckets = *reinterpret_cast<uint*>(&buffer[curr]);
    curr += sizeof(uint);

    //bucketOffset = std::stoi(params["bucketOffset"]);
	//bucketCount = std::stoi(params["bucketCount"]);
    uint indCount = 0;
    indexData = std::vector<std::vector<Coefficient>>(bucketCount);
    cout << "Parsing coefficients" << endl;
    for(long i = 0; i < bucketOffset; i++){
        result = fread (buffer,1,sizeof(uint),file);
        uint co = *reinterpret_cast<uint*>(&buffer[0]);
        curr += sizeof(uint) + (sizeof(uindex)+sizeof(float))*co;
        fseek(file, curr, SEEK_SET);
        //cout << co << " ";
    }
    cout << "\toffset " << bucketOffset <<  " jumped" << endl;
    cout << "\treading my " << bucketCount << " buckets" << endl;
    //cout << endl << nBuckets << " " << bucketOffset << " " << bucketCount << " " << curr << endl;
    for(long i = 0; i < bucketCount; i++){
        result = fread (buffer,1,sizeof(uint),file);
        uint co = *reinterpret_cast<uint*>(&buffer[0]);
        curr += sizeof(uint);

        if(debugLimitPerBucket == -1)
            indexData[i].resize(co);
        else
            indexData[i].resize(std::min((int)co,debugLimitPerBucket));

        indCount+=co;
        cout << "\t\tBucket " << i+bucketOffset << "( " << i << " +" << bucketOffset << ") has " << co << endl;

        for(uint j = 0; j < co; j++){
            result = fread (buffer,1,sizeof(uint),file);
            uindex gid = *reinterpret_cast<uindex*>(&buffer[0]);
            curr += sizeof(uindex);
            result = fread (buffer,1,sizeof(uint),file);
            float value = *reinterpret_cast<float*>(&buffer[0]);
            curr += sizeof(float);

            if(debugLimitPerBucket == -1 || ((int)j) < debugLimitPerBucket){

                auto search = lidTogidMap.find(gid);
                uindex lid = lidTogid.size();
                if(search != lidTogidMap.end())
                    lid = search->second;
                else {
                    lidTogidMap[gid] = lid;
                    lidTogid.push_back(gid);
                }


                indexData[i][j] = Coefficient(lid,value);

            }
        }
    }


    delete[] buffer;
    fclose(file);
    lidTogidMap.clear();

    cout << "Teste cap " << lidTogid.capacity() << endl;
    lidTogid.shrink_to_fit();
    cout << "Teste cap shrink " << lidTogid.capacity() << endl;

    cout << "Parsing coefficients... done" << endl;

    cout << "To import " << lidTogid.size() << " of " << indCount << " possible" << endl;

    cout << "Importing vectors" << endl;
    oneBillionImporterB ob;
    ob.readBin(dataPath,data,lidTogid);
    cout << "Importing vectors... done" << endl;

    cout << "Imported " << data.n_cols << endl;

    return true;
}

long getMem() {
    string cmd = "ps l " + std::to_string(::getpid()) + " | tail -1 | tr -s ' ' | cut -d ' ' -f 7";
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return std::stol(result);
}

template <typename T>
int SRProcessor<T>::loadB(string coeffs){
    map<uindex,uindex>* lidTogidMap = new map<uindex,uindex>();

    char* buffer = new char[sizeof(uint)];
    size_t result;

    uint indCount = 0;

    cout << "\treading my " << bucketCount << " buckets" << endl;
    //cout << endl << nBuckets << " " << bucketOffset << " " << bucketCount << " " << curr << endl;
    for(long i = 0; i < bucketCount; i++){
        uint loaded = 0;

        string currCoeff = coeffs + std::to_string(i+bucketOffset) + ".bin";
        FILE * file = fopen(currCoeff.c_str(), "rb" );
        //cout << currCoeff << endl;
        result = fread (buffer,1,sizeof(uint),file);
        uint co = *reinterpret_cast<uint*>(&buffer[0]);
        char* bufferCoeffs = new char[co*sizeof(Coefficient)];
        uint bufferCoeffsInd = 0;
        result = fread (bufferCoeffs,co,sizeof(Coefficient),file);
        fclose(file);


        if(debugLimitPerBucket == -1)
            indexData[i].reserve(co);
        else
            indexData[i].reserve(std::min((int)co,debugLimitPerBucket));

        indCount+=co;
        cout << "\t\tBucket " << i+bucketOffset << "( " << i << " +" << bucketOffset << ") has " << co << endl;
        //bufferCoeffsInd += sizeof(uint);

        for(uint j = 0; j < co; j++){
            uindex gid = *reinterpret_cast<uindex*>(&bufferCoeffs[bufferCoeffsInd]);
            bufferCoeffsInd += sizeof(uint);
            float value = *reinterpret_cast<float*>(&bufferCoeffs[bufferCoeffsInd]);
            bufferCoeffsInd += sizeof(float);
            if(maxIdToLoad == -1 || gid < maxIdToLoad){
                if(debugLimitPerBucket == -1 || ((int)j) < debugLimitPerBucket){
                    //loaded ++;
                    auto search = (*lidTogidMap).find(gid);
                    uindex lid = lidTogid.size();
                    if(search != (*lidTogidMap).end())
                        lid = search->second;
                    else {
                        (*lidTogidMap)[gid] = lid;
                        lidTogid.push_back(gid);
                    }
                    //cout << "a: " << debugLimitPerBucket << " " << maxIdToLoad << " " << lid << " " << gid << endl;
                    indexData[i].push_back(Coefficient(lid,value));
                }
            }
        }
        delete[] bufferCoeffs;

        //if(maxIdToLoad != -1){
        //cout << "Teste cap " << indexData[i].capacity() << endl;
        //indexData[i].shrink_to_fit();
        //cout << "Teste cap shrink " << indexData[i].capacity() << endl;

        //resize(loaded);
        //indexData[i].shrink_to_fit();
        //}

    }
    delete[] buffer;
    delete lidTogidMap;
    return indCount;
}

template <typename T>
bool SRProcessor<T>::loadBilionMultiFile(string coeffs, string dataPath){

    cout << "base: " << getMem()*1000 << endl;
    int baseMem = getMem()*1000;

    lidTogid.clear();
    indexData = std::vector<std::vector<Coefficient>>(bucketCount);



    cout << (getMem()*1000)-baseMem << endl;
    uint indCount = loadB(coeffs);
    cout << (getMem()*1000)-baseMem << endl;
    cout << "Teste cap " << lidTogid.capacity() << endl;
    lidTogid.shrink_to_fit();
    cout << "Teste cap shrink " << lidTogid.capacity() << endl;

    cout << (getMem()*1000)-baseMem  <<" " << lidTogid.size()*4+indCount*8 << endl;

    cout << "Parsing coefficients... done" << endl;

    cout << "To import " << lidTogid.size() << " of " << indCount << " possible" << endl;
    int memInter = (getMem()*1000)-baseMem;
    cout << "Importing vectors" << endl;
    oneBillionImporterB ob;
    ob.readBin(dataPath,data,lidTogid,2000000L,offsetDataFile);
    cout << "Importing vectors... done" << endl;

    cout << "Imported " << data.n_cols << endl;
    //long totalMem = data.n_cols*(long)128;
    //totalMem += lidTogid.size()*4+indCount*8;
    //cout << (getMem()*1000)-baseMem << " " << totalMem << " " << (getMem()*1000)-baseMem-memInter << " " << data.n_cols*(long)128  << endl;

    return true;
}



template <typename T>
bool SRProcessor<T>::load(string basePath){

    lidTogid.clear();
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

template <typename T>
int SRProcessor<T>::getClosestPivot(float coeff, vector<Coefficient>& bucket){
	uint imin = 0;
	uint imax = bucket.size()-1;
	uint imid = 0;
	while (imax >= imin && imin >= 0 && imax < bucket.size()){
		imid = imin + ((imax-imin)/2);

		float currentCoeff = bucket.at(imid).value;
		float diff = coeff-currentCoeff;
		//cout << "d: " << imin << " " << imid << " " << imax << " " << coeff << " " << currentCoeff << " " << diff << " " << bucket.at(imid).vector_pos << endl;
		if (abs(diff) < 0.00001)
			return imid;
        //else if (abs(lastDiff) < abs(diff))
        //    return bestMid;
		else if (diff > 0)
			imax = imid - 1;
		else
			imin = imid + 1;
	}
	return imid;
}


template class SRProcessor<int>;
template class SRProcessor<float>;
template class SRProcessor<uchar>;
