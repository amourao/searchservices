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
    bufferSize = std::stoi(params["bufferSize"]);
    baseFeatureExtractor = std::shared_ptr<FeatureExtractor>((FeatureExtractor*)FactoryAnalyser::getInstance()->createType(params["extractor"]));

    string serversString = params["servers"];
    vector<string> servers = StringTools::split(serversString,';');
    for (uint i = 0; i < servers.size(); i++){
        serverAddresses.push_back(Poco::Net::SocketAddress(servers.at(i)));
    }

    cout << "started Master at " << clientAddress.host().toString() << ":" << clientAddress.port() << endl;

}

SRMaster::~SRMaster(){

}

std::pair<vector<unsigned long>,vector<float> > SRMaster::knnSearchIdLong(arma::fmat& query, int n, float search_limit){

    unordered_set<unsigned long> indicesSet;
    vector<unsigned long> indices;
    vector<float> dists;

    QueryStructReq q;

    arma::fmat sparseRep;
    baseFeatureExtractor->extractFeatures(query,sparseRep);

    sparseRep= sparseRep.t();

    vector<Poco::Net::SocketAddress> servers = getRelevantServers(sparseRep);

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

    //#pragma omp parallel for schedule(dynamic)
    for(uint i = 0; i < servers.size(); i++){
        Poco::Net::SocketAddress server_address = servers.at(i);

        vector<QueryStructReq> input;
        vector<QueryStructRsp> output;

        input.push_back(q);
        //#pragma omp critical
        {
            sendMessage(input,output,server_address);
        }
        if(output.size() == 0){
            cout << "Master " << " failed to receive response from " << server_address.host().toString() << ":" << server_address.port() << endl;
        }
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

    auto compare = [](float a, float b){ return a < b; };
    auto p = MatrixTools::sortPermutation(dists,compare);

    dists = MatrixTools::applyPermutation(dists, p, n);
    indices = MatrixTools::applyPermutation(indices, p, n);

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

void SRMaster::sendMessage(vector<QueryStructReq>& query, vector<QueryStructRsp>& output, Poco::Net::SocketAddress& server){

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

    Poco::Net::DatagramSocket dgs(clientAddress);
    dgs.setReceiveTimeout(Poco::Timespan(1000*10));
    cout << "Master " << clientAddress.host().toString() << ":" << clientAddress.port()  << " wants to send " << totalSize << " to " << server.host().toString() << ":" << server.port() << endl;

    cout << clientAddress.host().toString() << " " << clientAddress.port() << endl;
    int sent = dgs.sendTo(&inbuffer[0], totalSize, server);
    cout << "Master " << clientAddress.host().toString() << ":" << clientAddress.port()  << " sent " << sent << " to " << server.host().toString() << ":" << server.port() << endl;

    //std::ofstream outfile ("master.bin",std::ofstream::binary);
    //outfile.write (&inbuffer[0],totalSize);
    //outfile.close();



    int floatBufferSize = 0;
    char* outputBytes = new char[bufferSize];

    try {
        floatBufferSize = dgs.receiveBytes(outputBytes,bufferSize);
        cout << "Master " << clientAddress.host().toString() << ":" << clientAddress.port()  << " received " << floatBufferSize << " from " << server.host().toString() << ":" << server.port() << endl;

        uint numOpsRsp = (uint)outputBytes[0];
        uint accumBytes = 1;

        for (uint i = 0; i < query.size(); i++){
            char* newInput = &outputBytes[accumBytes];

            QueryStructRsp response;
            response.toQueryStructRsp(newInput);

            char op = response.operation;
            accumBytes += response.totalByteSize;

            output.push_back(response);
        }

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
