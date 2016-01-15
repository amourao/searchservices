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

    cout << "started client at " << clientAddress.host().toString() << ":" << clientAddress.port() << endl;

}

SRMaster::~SRMaster(){

}

std::pair<vector<unsigned long>,vector<float> > SRMaster::knnSearchIdLong(arma::fmat& query, int n, float search_limit){

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

    for(uint i = 0; i < servers.size(); i++){
        Poco::Net::SocketAddress server_address = servers.at(i);

        vector<QueryStructReq> input;
        vector<QueryStructRsp> output;

        input.push_back(q);

        sendMessage(input,output,server_address);

        //int current_i = 2;
        for(uint s = 0; s < output.size(); s++){
            indices.insert(indices.end(), output[s].indexes.begin(), output[s].indexes.end());
            dists.insert(dists.end(), output[s].dists.begin(), output[s].dists.end());
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
    cout << "Master wants to send " <<  totalSize << endl;

    cout << clientAddress.host().toString() << " " << clientAddress.port() << endl;
    int sent = dgs.sendTo(&inbuffer[0], totalSize, server);
    cout << "Master " << clientAddress.host().toString() << ":" << clientAddress.port()  << " sent " << sent << " to " << server.host().toString() << ":" << server.port() << endl;

    //std::ofstream outfile ("master.bin",std::ofstream::binary);
    //outfile.write (&inbuffer[0],totalSize);
    //outfile.close();

    int floatBufferSize = 0;
    char* outputBytes = new char[bufferSize];
    floatBufferSize = dgs.receiveBytes(outputBytes,bufferSize);
    cout << "Master " << clientAddress.host().toString() << ":" << clientAddress.port()  << " received " << floatBufferSize << " from " << server.host().toString() << ":" << server.port() << endl;

    uint numOpsRsp = (uint)outputBytes[0];
    uint accumBytes = 1;

    for (uint i = 0; i < numOps; i++){
        char* newInput = &outputBytes[accumBytes];

        QueryStructRsp response;
        response.toQueryStructRsp(newInput);

        char op = response.operation;
        accumBytes += response.totalByteSize;

        output.push_back(response);
    }
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

    cout << "relevant servers ";
    for(uint i = 0; i < results.size(); i++){
        cout << resultsIndex.at(i) << " ";
    }
    cout << endl;

    return results;

}
