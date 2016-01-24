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

    clientAddress = Poco::Net::SocketAddress("0.0.0.0", std::stoi(params["port"]));
    uint basePort = std::stoi(params["port"]);

    bufferSize = std::stoi(params["bufferSize"]);
    baseFeatureExtractor = std::shared_ptr<FeatureExtractor>((FeatureExtractor*)FactoryAnalyser::getInstance()->createType(params["extractor"]));

    string serversString = params["servers"];
    vector<string> servers = StringTools::split(serversString,';');
    for (uint i = 0; i < servers.size(); i++){
        serverAddresses.push_back(Poco::Net::SocketAddress(servers.at(i)));
        clientAddresses.push_back(Poco::Net::SocketAddress("0.0.0.0", basePort++));
    }

    cout << "started Master at " << clientAddress.host().toString() << ":" << clientAddress.port() << endl;

}

SRMaster::~SRMaster(){

}

std::pair<vector<uindex>,vector<float> > SRMaster::knnSearchIdLong(arma::fmat& query, int n, float search_limit){
    #ifdef MEASURE_TIME
        totalNQueries++;
        totalQueryTimeStart = NOW();
        totalSRTimeStart = NOW();
    #endif

    unordered_set<uindex> indicesSet;
    vector<uindex> indices;
    vector<float> dists;

    QueryStructReq q;

    arma::fmat sparseRep;
    baseFeatureExtractor->extractFeatures(query,sparseRep);

    sparseRep = sparseRep.t();

    #ifdef MEASURE_TIME
        totalSRTime += ELAPSED(totalSRTimeStart);
        totalRelServerTimeStart = NOW();
    #endif

    vector<Poco::Net::SocketAddress> servers = getRelevantServers(sparseRep);


    #ifdef MEASURE_TIME
        totalRelServerTime += ELAPSED(totalRelServerTimeStart);
        totalPreMarshallingTimeStart = NOW();
    #endif

    float* arrayData = (float*)&query(0);
    q.query.insert(q.query.end(), arrayData, arrayData+query.n_rows);
    q.parameters.push_back(n);
    q.parameters.push_back(search_limit);
    q.operation = 1;


    for(uint i = 0; i < sparseRep.n_rows; i++){
        if(sparseRep(i,0) != 0){
            q.buckets.push_back(i);
            q.coeffs.push_back(sparseRep(i,0));
        }
    }

    #ifdef MEASURE_TIME
        totalRequests+=servers.size();
        totalPreMarshallingTime += ELAPSED(totalPreMarshallingTimeStart);
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
            #pragma omp critical
            {
                #ifdef MEASURE_TIME
                    missedPackages++;
                #endif
            }
            cout << "Master " << client_address.host().toString() << ":" << client_address.port() << " failed to receive response from " << server_address.host().toString() << ":" << server_address.port() << endl;
        }

        #pragma omp critical
        {
            output.insert(output.end(),outputIn.begin(),outputIn.end());
        }

    }

    #ifdef MEASURE_TIME
        totalAllServerTime += ELAPSED(totalAllServerTimeStart);
        totalPreMarshallingTimeStart = NOW();
    #endif

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
    #ifdef MEASURE_TIME
        totalPreMarshallingTime += ELAPSED(totalPreMarshallingTimeStart);
    #endif



    #ifdef MEASURE_TIME
            totalSortTimeStart = NOW();
    #endif

    auto compare = [](float a, float b){ return a < b; };
    auto p = MatrixTools::sortPermutation(dists,compare);

    dists = MatrixTools::applyPermutation(dists, p, n);
    indices = MatrixTools::applyPermutation(indices, p, n);

    #ifdef MEASURE_TIME
            totalQueryTime += ELAPSED(totalQueryTimeStart);
            totalSortTime += ELAPSED(totalSortTimeStart);
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
        tp _totalMarshallingTimeStart = NOW();
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
        totalMarshallingTime += ELAPSED(_totalMarshallingTimeStart);
        tp _totalCommunicationTimeStart = NOW();
        tp _totalCommunicationSendTimeStart = NOW();
    #endif

    Poco::Net::DatagramSocket dgs(client);
    dgs.setReceiveTimeout(Poco::Timespan(1000*10));
    cout << "Master " << client.host().toString() << ":" << client.port()  << " wants to send " << totalSize << " to " << server.host().toString() << ":" << server.port() << endl;

    //cout << clientAddress.host().toString() << " " << clientAddress.port() << endl;
    int sent = dgs.sendTo(&inbuffer[0], totalSize, server);
    cout << "Master " << client.host().toString() << ":" << client.port()  << " sent " << sent << " to " << server.host().toString() << ":" << server.port() << endl;

    #ifdef MEASURE_TIME
        totalCommunicationSendTime += ELAPSED(_totalCommunicationSendTimeStart);
        tp _totalCommunicationReceiveTimeStart = NOW();
    #endif
    //std::ofstream outfile ("master.bin",std::ofstream::binary);
    //outfile.write (&inbuffer[0],totalSize);
    //outfile.close();

    int floatBufferSize = 0;
    char* outputBytes = new char[bufferSize];

    try {
        floatBufferSize = dgs.receiveBytes(outputBytes,bufferSize);
        cout << "Master " << client.host().toString() << ":" << client.port()  << " received " << floatBufferSize << " from " << server.host().toString() << ":" << server.port() << endl;
        #ifdef MEASURE_TIME
            totalCommunicationReceiveTime += ELAPSED(_totalCommunicationReceiveTimeStart);
            totalCommunicationTime += ELAPSED(_totalCommunicationTimeStart);
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
            totalMarshallingTime += ELAPSED(_totalMarshallingTimeStart);
        #endif

    } catch(Poco::TimeoutException t){

    }
    delete[] outputBytes;
    //float* floatBuffer = reinterpret_cast<float*>(buffer);
	//output.insert(output.end(), &buffer[0], &buffer[floatBufferSize]);
}


vector<Poco::Net::SocketAddress> SRMaster::getRelevantServers(arma::fmat& sparseRep){
    vector<Poco::Net::SocketAddress> results;
    vector<int> resultsIndex;

    uint repSize = sparseRep.n_rows;
    uint bucketsPerServer = repSize/(float)serverAddresses.size();
    for(uint i = 0; i < serverAddresses.size()-1; i++){
        arma::fmat subset = sparseRep.rows(i*bucketsPerServer,(i+1)*bucketsPerServer);
        arma::uvec b = find(subset > 0);
        uint count = b.n_rows;
        if(count > 0){
            results.push_back(serverAddresses.at(i));
            resultsIndex.push_back(i);
        }
    }
    uint i = serverAddresses.size()-1;
    arma::fmat subset = sparseRep.rows(i*bucketsPerServer,min((i+1)*bucketsPerServer,(uint)sparseRep.n_rows-1));
    arma::uvec b = find(subset > 0);
    int count = b.n_rows;
    if(count > 0){
        results.push_back(serverAddresses.at((serverAddresses.size()-1)));
        resultsIndex.push_back(serverAddresses.size()-1);
    }

    string relServers = "Relevant servers: ";
    for(uint i = 0; i < results.size(); i++){
        relServers += std::to_string(resultsIndex.at(i)) + " ";
    }
    cout << relServers << endl;

    return results;

}

void SRMaster::printProcessorsStatistics(){
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
                cout << output.at(0).indexes.at(j) << endl;
            }
        }
    }
}
