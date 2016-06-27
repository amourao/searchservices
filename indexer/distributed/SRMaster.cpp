#include "SRMaster.h"

//static SRMaster distIndexerWrapperClientFactoryDist;
//static int distIndexerWrapperClientFactoryDistPortCount = 0;

SRMaster::SRMaster(){
	//FactoryIndexer::getInstance()->registerType("SRMaster",this);
}

SRMaster::SRMaster(string& typeId, map<string,string>& params){
    type = typeId;
    paramsB = params;

    if(params.size() == 0){
        return;
    }


    timeoutTime = std::stoi(params["timeoutTime"]);

    clientAddress = Poco::Net::SocketAddress("0.0.0.0", std::stoi(params["port"]));
    uint basePort = std::stoi(params["port"]);

    bufferSize = std::stoi(params["bufferSize"]);
    baseFeatureExtractor = std::shared_ptr<FeatureExtractor>((FeatureExtractor*)FactoryAnalyser::getInstance()->createType(params["extractor"]));

    string serversString = params["servers"];

    if(params.count("startFromPivot") > 0)
        startFromPivot = true;

    if(params.count("doFinalSort") > 0)
        doFinalSort = true;

    if(params.count("doFinalSortCoeff") > 0)
        doFinalSortCoeff = true;

    posOnly = params.find("positiveOnly") != params.end();

    vector<string> servers = StringTools::split(serversString,';');
    for (uint i = 0; i < servers.size(); i++){
        serverAddresses.push_back(Poco::Net::SocketAddress(servers.at(i)));
        clientAddresses.push_back(Poco::Net::SocketAddress("0.0.0.0", basePort++));
    }

    if(params.count("bucketTransposition") > 0){
        std::ifstream infile(params["bucketTransposition"]);
        uint a;
        while (infile >> a){
            bucketTransposition.push_back(a);
        }
    }

    cout << "--------------------------------------------------------------------------------" << endl;
    cout << "started Master at " << clientAddress.host().toString() << ":" << clientAddress.port() << endl;

}

SRMaster::~SRMaster(){

}


std::pair<std::vector<uindex>,std::vector<float>> SRMaster::computeScores(const std::vector<uindex>& ind, const std::vector<float>& dists){
    std::map<uindex,std::pair<uint,float>> scores;
    std::vector<uindex> nind;
    std::vector<float> ndists;

    for(uint i = 0; i < ind.size(); i++){
        auto it = scores.find(ind[i]);
        if (it == scores.end()){
            scores[ind[i]] = make_pair(1,dists[i]);
        } else {
            scores[ind[i]] = make_pair(it->second.first+1,it->second.second + dists[i]);
        }
    }

    for(auto& it : scores){
        nind.push_back(it.first);
        ndists.push_back(-(it.second.first-it.second.second) );
    }
    return make_pair(nind,ndists);
}


std::pair<vector<uindex>,vector<float> > SRMaster::knnSearchIdLong(arma::fmat& query, int n, float search_limit){
    #ifdef MEASURE_TIME
        unsigned long totalQueryTimeQ = 0;
        unsigned long totalSRTimeQ = 0;
        unsigned long totalRelServerTimeQ = 0;
        unsigned long totalPreMarshallingTimeQ = 0;
        unsigned long totalAllServerTimeQ = 0;
        unsigned long totalPostMarshallingTimeQ = 0;
        unsigned long totalSortTimeQ = 0;

        totalNQueries++;
        totalQueryTimeStart = NOW();
        totalSRTimeStart = NOW();
    #endif

    unordered_set<uindex> indicesSet;
    vector<uindex> indices;
    vector<float> dists;

    QueryStructReq q;

    arma::fmat sparseRep;

    arma::fmat query2 = query;
    normalizeColumns(query2);
    baseFeatureExtractor->extractFeatures(query2,sparseRep);

    sparseRep = sparseRep.t();

    #ifdef MEASURE_TIME
        totalSRTimeQ = ELAPSED(totalSRTimeStart);
        totalSRTime += totalSRTimeQ;
        
        totalRelServerTimeStart = NOW();
    #endif

    vector<Poco::Net::SocketAddress> servers = getRelevantServers(sparseRep);


    #ifdef MEASURE_TIME
    
        
        totalRelServerTimeQ = ELAPSED(totalRelServerTimeStart);
        totalRelServerTime += totalRelServerTimeQ;

        totalPreMarshallingTimeStart = NOW();
    #endif

    float* arrayData = (float*)&query(0);
    q.query.insert(q.query.end(), arrayData, arrayData+query.n_rows);
    q.parameters.push_back(n);
    q.parameters.push_back(search_limit);
    q.parameters.push_back(totalNQueries-1);

    q.parameters.push_back((int)startFromPivot);
    q.parameters.push_back((int)doFinalSort);
    q.parameters.push_back((int)doFinalSortCoeff);

    q.operation = 1;

    float coeffSum = 0;
    for(uint i = 0; i < sparseRep.n_rows; i++){
        if( (posOnly && sparseRep(i,0) > 0) ||  (!posOnly && sparseRep(i,0) != 0) ){
            q.buckets.push_back(i);
            q.coeffs.push_back(sparseRep(i,0));
            coeffSum+=abs(sparseRep(i,0));
        }
    }
    q.parameters.push_back(coeffSum);
    
    #ifdef MEASURE_TIME
        totalRequests+=servers.size();

        totalPreMarshallingTimeQ = ELAPSED(totalPreMarshallingTimeStart);
        totalPreMarshallingTime += totalPreMarshallingTimeQ; 
        totalAllServerTimeStart = NOW();
    #endif


    vector<QueryStructRsp> output;

    #pragma omp parallel for schedule(dynamic)
    for(uint i = 0; i < servers.size(); i++){
        Poco::Net::SocketAddress server_address = servers.at(i);
        Poco::Net::SocketAddress client_address = clientAddresses.at(i);

        vector<QueryStructReq> input;
        vector<QueryStructRsp> outputIn;

        input.push_back(q);
        //#pragma omp critical
        {
            sendMessage(input,outputIn,client_address,server_address);
        }
        if(outputIn.size() == 0){
            #ifdef MEASURE_TIME
                #pragma omp critical
                {
                        missedPackages++;
                }
            #endif
            cout << "Master " << client_address.host().toString() << ":" << client_address.port() << " failed to receive response from " << server_address.host().toString() << ":" << server_address.port() << endl;
        }

        #pragma omp critical
        {
            output.insert(output.end(),outputIn.begin(),outputIn.end());
        }

    }

    #ifdef MEASURE_TIME
        
        totalAllServerTimeQ = ELAPSED(totalAllServerTimeStart);
        totalAllServerTime += totalAllServerTimeQ;

        totalPostMarshallingTimeStart = NOW();
    #endif
    if(!doFinalSortCoeff){
        for(uint s = 0; s < output.size(); s++){
            for(uint p = 0; p < output[s].indexes.size(); p++){
                if (indicesSet.find(output[s].indexes[p]) == indicesSet.end()){
                    indicesSet.insert(output[s].indexes[p]);
                    indices.push_back(output[s].indexes[p]);
                    dists.push_back(output[s].dists[p]);
                    //dists.insert(dists.end(), output[s].dists.begin(), output[s].dists.end());
                }
            }
        }
    }
    else{
        for(uint s = 0; s < output.size(); s++){
            for(uint p = 0; p < output[s].indexes.size(); p++){
                indices.push_back(output[s].indexes[p]);
                dists.push_back(output[s].dists[p]);
            }
        }
        std::pair<std::vector<uindex>,std::vector<float>> pp = computeScores(indices,dists);
        indices = pp.first;
        dists = pp.second;

    }
    #ifdef MEASURE_TIME
        totalPostMarshallingTimeQ = ELAPSED(totalPostMarshallingTimeStart);
        totalPostMarshallingTime += totalPostMarshallingTimeQ;
        totalSortTimeStart = NOW();
    #endif

    auto compare = [](float a, float b){ return a < b; };
    auto p = MatrixTools::sortPermutation(dists,compare);

    dists = MatrixTools::applyPermutation(dists, p, n);
    indices = MatrixTools::applyPermutation(indices, p, n);

    #ifdef MEASURE_TIME
            totalSortTimeQ = ELAPSED(totalSortTimeStart);
            totalQueryTimeQ =  ELAPSED(totalQueryTimeStart);
            totalQueryTime += totalQueryTimeQ;
            totalSortTime += totalSortTimeQ;
    #endif

    #ifdef MEASURE_TIME
        cout << "T1;" <<  totalNQueries-1 << ";" << (totalQueryTimeQ) << ";" << (totalSRTimeQ) << ";" << (totalRelServerTimeQ) << ";" << (totalPreMarshallingTimeQ) << ";" << (totalAllServerTimeQ) << ";" << (totalPostMarshallingTimeQ) << ";" << (totalSortTimeQ) << endl; 
    #endif

	return make_pair(indices,dists);
}

bool SRMaster::save(string basePath){
	return true;
}

bool SRMaster::load(string basePath){
	return true;
}

string SRMaster::getName(){
	return type;
}

int SRMaster::addToIndexLive(arma::fmat& features){
    return 0;
}

void SRMaster::sendMessage(vector<QueryStructReq>& query, vector<QueryStructRsp>& output, Poco::Net::SocketAddress& client, Poco::Net::SocketAddress& server){

    #ifdef MEASURE_TIME

        unsigned long totalReqTimeQ = 0;
        unsigned long totalMarshallingTimeQ = 0;
        unsigned long totalRequestComunicationTimeQ = 0;
        unsigned long totalRequestComunicationSendTimeQ = 0;
        unsigned long totalRequestComunicationRecTimeQ = 0;
        unsigned long totalPreMarshallingTimeQ = 0;
        unsigned long totalAllServerTimeQ = 0;
        unsigned long totalPostMarshallingTimeQ = 0;
        unsigned long totalSortTimeQ = 0;

        tp _totalMarshallingTimeStart = NOW();
        tp _totalReqTimeStart = NOW();

        
    #endif

    char numOps = (char)query.size();
    uint totalSize = 1;
    for (uint i = 0; i < query.size(); i++){
        query[i].totalByteSize = query[i].computeTotalByteSize();
        totalSize+=query[i].totalByteSize;
    }
    char* inbuffer = new char[totalSize];
    inbuffer[0] = numOps;

    uint curByte = 1;
    for (uint i = 0; i < query.size(); i++){
        char* tmp = query[i].toBytes();
        memcpy(&inbuffer[curByte],&tmp[0],query[i].totalByteSize);
        curByte+=query[i].totalByteSize;
    }
    #ifdef MEASURE_TIME
        totalMarshallingTimeQ = ELAPSED(_totalMarshallingTimeStart);
        totalMarshallingTime += totalMarshallingTimeQ;

        tp _totalCommunicationTimeStart = NOW();
        tp _totalCommunicationSendTimeStart = NOW();
    #endif

    Poco::Net::DatagramSocket dgs(client);
    dgs.setReceiveTimeout(Poco::Timespan(1000*timeoutTime));
    //cout << "Ma " << client.host().toString() << ":" << client.port()  << " send " << totalSize << " to " << server.host().toString() << ":" << server.port() << endl;

    //cout << clientAddress.host().toString() << " " << clientAddress.port() << endl;
    int sent = dgs.sendTo(&inbuffer[0], totalSize, server);
    //cout << "Master " << client.host().toString() << ":" << client.port()  << " sent " << sent << " to " << server.host().toString() << ":" << server.port() << endl;

    #ifdef MEASURE_TIME
        totalRequestComunicationSendTimeQ = ELAPSED(_totalCommunicationSendTimeStart);
        
        totalCommunicationSendTime += totalRequestComunicationSendTimeQ;
        tp _totalCommunicationReceiveTimeStart = NOW();
    #endif
    //std::ofstream outfile ("master.bin",std::ofstream::binary);
    //outfile.write (&inbuffer[0],totalSize);
    //outfile.close();

    int floatBufferSize = 0;
    char* outputBytes = new char[bufferSize];

    try {
        floatBufferSize = dgs.receiveBytes(outputBytes,bufferSize);
        //cout << "Ma " << client.host().toString() << ":" << client.port()  << " recv " << floatBufferSize << " from " << server.host().toString() << ":" << server.port() << endl;
        #ifdef MEASURE_TIME
            totalRequestComunicationTimeQ = ELAPSED(_totalCommunicationTimeStart);
            totalCommunicationTime += totalRequestComunicationTimeQ;

            totalRequestComunicationRecTimeQ = ELAPSED(_totalCommunicationReceiveTimeStart);
            totalCommunicationReceiveTime += totalRequestComunicationRecTimeQ; 
            
            tp _totalMarshallingTimeStart = NOW();
        #endif
        //uint numOpsRsp = (uint)outputBytes[0];
        uint accumBytes = 1;

        for (uint i = 0; i < query.size(); i++){
            char* newInput = &outputBytes[accumBytes];

            QueryStructRsp response;
            response.toQueryStructRsp(newInput);

            //char op = response.operation;
            accumBytes += response.totalByteSize;

            output.push_back(response);
        }

        #ifdef MEASURE_TIME
            totalReqTimeQ = ELAPSED(_totalReqTimeStart);
            totalPostMarshallingTimeQ = ELAPSED(_totalMarshallingTimeStart);
            totalMarshallingTime += totalPostMarshallingTimeQ;
        #endif


    } catch(Poco::TimeoutException t){

    }

    #ifdef MEASURE_TIME
        if(query.size() > 0 && query[0].parameters.size() > 2)
            cout << "T2;" <<  query[0].parameters[2] << ";" << (totalReqTimeQ) << ";" << (totalMarshallingTimeQ) << ";" << (totalRequestComunicationTimeQ) << ";" << (totalRequestComunicationSendTimeQ) << ";" << (totalRequestComunicationRecTimeQ) << ";" << (totalPreMarshallingTimeQ) << ";" << (totalAllServerTimeQ) << ";" << (totalPostMarshallingTimeQ) << ";" << (totalSortTimeQ) << endl; 
    #endif
    delete[] outputBytes;
    //float* floatBuffer = reinterpret_cast<float*>(buffer);
	//output.insert(output.end(), &buffer[0], &buffer[floatBufferSize]);
}


vector<Poco::Net::SocketAddress> SRMaster::getRelevantServers(arma::fmat& sparseRep){
    vector<Poco::Net::SocketAddress> results;
    vector<int> resultsIndex;

    uint repSize = sparseRep.n_rows;
    uint bucketsPerServer = repSize/(float)serverAddresses.size();
    for(uint i = 0; i < serverAddresses.size(); i++){
        arma::fmat subset = sparseRep.rows(i*bucketsPerServer, min((i+1)*bucketsPerServer,(uint)sparseRep.n_rows)-1 );

        arma::uvec b;
        if(posOnly)
            b = find(subset > 0);
        else
            b = find(subset != 0);

        uint count = b.n_rows;
        if(count > 0){
            int bucket = i;
            if(bucketTransposition.empty())
                bucket = bucketTransposition[bucket];
            results.push_back(serverAddresses.at(bucket));
            resultsIndex.push_back(bucket);
        }
    }
        
    string relServers = "Rel Serv: ";
    for(uint i = 0; i < results.size(); i++){
        relServers += std::to_string(resultsIndex.at(i)) + " ";
    }
    cout << relServers << endl;

    return results;

}

void SRMaster::printProcessorsStatistics(){
    vector<string> statistics;
    statistics.push_back("totalTime");
    statistics.push_back("\ttotalQueryTime");
    statistics.push_back("\ttotalCommunicationReceiveTime");
    statistics.push_back("\ttotalCommunicationSendTime");
    statistics.push_back("\ttotalBucketTime");
    statistics.push_back("\ttotalSortTime");
    statistics.push_back("\ttotalPreMarshallingTime");
    statistics.push_back("\ttotalMarshallingTime");
    statistics.push_back("totalNQueries");
    statistics.push_back("totalNBucketsReq");
    statistics.push_back("totalNBucketInsp");
    statistics.push_back("totalNCandidatesInsp");
    statistics.push_back("totalNCandidatesInspNonDup");
    statistics.push_back("missedPackages");

    vector<QueryStructReq> query;

    QueryStructReq q;
    q.operation = 10;

    query.push_back(q);

    for(uint i = 0; i < serverAddresses.size(); i++){
        vector<QueryStructRsp> output;
        Poco::Net::SocketAddress server = serverAddresses.at(i);
        Poco::Net::SocketAddress client = clientAddresses.at(i);
        sendMessage(query, output, client, server);
        cout << "Server statistics: " << server.host().toString() << ":" << server.port() << endl;
        if(output.size() > 0){
            for(uint j = 0; j < output.at(0).indexes.size(); j++){
                cout << statistics.at(j) << ": " <<  output.at(0).indexes.at(j) << endl;
            }
        }
    }
}
