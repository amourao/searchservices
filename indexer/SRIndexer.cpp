#include "SRIndexer.h"

static SRIndexer srIndexerFactory;

SRIndexer::SRIndexer(){
	FactoryIndexer::getInstance()->registerType("srIndexer",this);
}

SRIndexer::SRIndexer(string& typeId){
    type = typeId;
}

SRIndexer::SRIndexer(string& typeId, map<string,string>& params){

    /*
    struct DALMOptions {
        uword max_iters; // 10000
        double tolerance; //1e-3
        double lambda;
        double beta;
    };

    struct FISTAOptions {
        uword max_iters;
        double L;
        double tolerance;
        double lambda;
    };

    struct OMPSparseConstrainedOptions {
        uword max_iters;
        double eps;
    };

    struct ThresholdingOptions {
        uword max_iters;
    };
    */

    dimensions = std::stoi(params["dimensions"]);
    n_iter = std::stoi(params["n_iter"]);
    search_limit = std::stoi(params["search_limit"]);
    createNewLNReconstructor(params);


    type = typeId;
    paramsB = params;
}


void SRIndexer::createNewLNReconstructor(map<string,string>& params){

    params >> opt;

    lnMinKSVD = lnminK_Ptr(new lnminK(opt));
    lnMinQuery = lnmin_Ptr(new lnmin(opt));

    params >> optKSVD;
}


SRIndexer::~SRIndexer(){

}

void* SRIndexer::createType(string &typeId, map<string,string>& params){
    return new SRIndexer(typeId,params);
}

void* SRIndexer::createType(string &typeId){
	if (typeId == "srIndexer"){
		return new SRIndexer(typeId);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void SRIndexer::train(arma::fmat& featuresTrain,arma::fmat& featuresValidationI,arma::fmat& featuresValidationQ){
	dictionary = arma::randu<arma::fmat>(featuresTrain.n_rows, dimensions);
    utils::normalize_columns(dictionary);
    ksvd = ksvdb_Ptr(new ksvdb(optKSVD,*lnMinKSVD,dictionary,featuresTrain));

    for (int i = 0; i < n_iter; i++) {
        ksvd->iterate();
    }

}

void SRIndexer::indexWithTrainedParams(arma::fmat& features){
    indexData = std::make_shared<fmat>(features);
    indexKSVD = indexk_Ptr( new indexk(*lnMinQuery, ksvd->D));
    indexKSVD->load(indexData, lnMinQuery->options.max_iters);

}

void SRIndexer::index(arma::fmat& features){

}

std::pair<vector<float>,vector<float> > SRIndexer::knnSearchId(arma::fmat& query, int n){
	vector<float> indices (n);
	vector<float> dists (n);

    auto ksvd_res = indexKSVD->find_k_nearest_limit(query, n, search_limit);

    for (auto& res : ksvd_res) {
        indices.push_back(res.vector_pos);
        dists.push_back(res.value);
    }

	return make_pair(indices,dists);
}

std::pair<vector<float>,vector<float> > SRIndexer::radiusSearchId(arma::fmat& query, double radius, int n){
	vector<float> indices (n);
	vector<float> dists (n);
	return make_pair(indices,dists);
}

bool SRIndexer::save(string basePath){

    indexKSVD->save(basePath);

    Json::Value root;

    for (auto& kv : paramsB) {
        root[kv.first] =  kv.second;
    }

    std::fstream file (basePath+".json", std::fstream::in | std::fstream::out);

    file << root;
    file.close();

	return true;
}

bool SRIndexer::load(string basePath){

    Json::Value root;
    Json::Reader reader;

    std::fstream file (basePath+".json", std::fstream::in | std::fstream::out);

    reader.parse( file, root );

    paramsB = jsonToDict(root);

    dimensions = std::stoi(paramsB["dimensions"]);
    n_iter = std::stoi(paramsB["n_iter"]);
    search_limit = std::stoi(paramsB["search_limit"]);
    createNewLNReconstructor(paramsB);

	return true;
}

string SRIndexer::getName(){
	return type;
}


map<string,string> SRIndexer::jsonToDict(Json::Value root){
    map<string,string> result;
    for( Json::ValueIterator itr = root.begin(); itr != root.end(); itr++ ){
        if (root[itr.key().asString()].isString()){
            result[itr.key().asString()] = root[itr.key().asString()].asString();
        }
    }
    return result;
}
