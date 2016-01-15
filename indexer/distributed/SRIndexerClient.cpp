#include "SRIndexerClient.h"

//static SRIndexerClient distIndexerWrapperServerFactory;

SRIndexerClient::SRIndexerClient(){
    //FactoryIndexer::getInstance()->registerType("SRIndexerClient",this);
}

SRIndexerClient::SRIndexerClient(string& typeId, map<string,string>& params){
    type = typeId;
    paramsB = params;
}

SRIndexerClient::~SRIndexerClient(){

}

void SRIndexerClient::index(QueryStructReq queryS){
    
}

QueryStructRsp SRIndexerClient::knnSearchIdLong(QueryStructReq queryS){
    
    arma::fmat query(&queryS.query[0],1,queryS.size(),false);
    vector<int> buckets = queryS.buckets;
    int n = queryS.params[0];
    double search_limit = queryS.params[1];

    QueryStructRsp result;

    int on = std::min(n,(int)(indexData.size()*search_limit));

    vector<Coefficient> candidates(indexData.begin(),indexData.begin()+on);
    vector<unsigned long> indices(on);
    vector<float> dists (on);

    for(int i = 0; i < on; i++){
        float dist = norm(data.col(candidates[i].vector_pos) - query, 2);
        long index = candidates[i].original_id;

        indices[i] = index;
        dists[i] = dist;
    }

    auto compare = [](float a, float b){ return a < b; };
    auto p = MatrixTools::sortPermutation(dists,compare);

    result.dists = MatrixTools::applyPermutation(dists, p,(uint)n);
    result.indexes = MatrixTools::applyPermutation(indices, p,(uint)n);

    return result;
}

bool SRIndexerClient::save(string basePath){
    return true;
}

bool SRIndexerClient::load(string basePath){
    return true;
}

