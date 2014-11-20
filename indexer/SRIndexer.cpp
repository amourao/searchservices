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
    if (params.size() > 0){
        dimensions = std::stoi(params["dimensions"]);
        n_iter = std::stoi(params["n_iter"]);

        max_iters = std::stoi(params["max_iters_ksvd"]);
        eps = std::stod(params["eps_ksvd"]);
        //index_path = paramsB["index_path"];
        createNewLNReconstructor(params);
        //load(paramsB["index_path"]);

        trainDataSize = SR_DEFAULT_TRAIN_DATA_SIZE;
        if(params.count("trainDataSize") > 0)
            trainDataSize = std::stoi(params["trainDataSize"]);

        type = typeId;
        paramsB = params;
    }
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
    featuresTrain = featuresTrain.t();
    featuresValidationI = featuresValidationI.t();
    featuresValidationQ = featuresValidationQ.t();

    trainDataSize = featuresTrain.n_cols;

	dictionary = arma::randu<arma::fmat>(featuresTrain.n_rows, dimensions);
    utils::normalize_columns(dictionary);
    ksvd = ksvdb_Ptr(new ksvdb(optKSVD,*lnMinKSVD,dictionary,featuresTrain));

    for (int i = 0; i < n_iter; i++) {
        std::cout << "Iteration " << i+1 << std::endl;
        ksvd->iterate();
    }

}

void SRIndexer::indexWithTrainedParams(arma::fmat& features){
    index(features);
}

void SRIndexer::index(arma::fmat& features){

    features = features.t();

    arma::fmat trainSplit;
    vector<arma::fmat> in;
    vector<arma::fmat> out;

    in.push_back(features);

    MatrixTools::getRandomSample(in,trainDataSize,out);

    trainSplit = out.at(0);

    train(trainSplit,trainSplit,trainSplit);

    indexData = std::make_shared<fmat>(features);
    //TODO
    /*
    fmat dictionary = randu<fmat>(features.n_rows, dimensions);
    utils::normalize_columns(dictionary);

    typename sparse_trait<Algorithm>::ksvd::option_type ksvd_options;
    ksvd_options.max_iters = iters;
    ksvd_options.eps       = eps;
    */
    indexKSVD = indexk_Ptr( new indexk(*lnMinQuery, ksvd->D));
    indexKSVD->load(indexData, lnMinQuery->options.max_iters);

}

std::pair<vector<float>,vector<float> > SRIndexer::knnSearchId(arma::fmat& query, int n, double search_limit){
    query = query.t();

	vector<float> indices;
	vector<float> dists;

    auto ksvd_res = indexKSVD->find_k_nearest_limit(query, n, search_limit*indexKSVD->size());

    for (auto& res : ksvd_res) {
        indices.push_back(res.vector_pos);
        dists.push_back(std::pow(res.value,2));
    }

	return make_pair(indices,dists);
}

std::pair<vector<float>,vector<float> > SRIndexer::radiusSearchId(arma::fmat& query, double radius, int n, double search_limit){
    query = query.t();

	vector<float> indices (n);
	vector<float> dists (n);
	return make_pair(indices,dists);
}

bool SRIndexer::save(string basePath){

    saveLabels(basePath);
    indexKSVD->save(INDEXER_BASE_SAVE_PATH + basePath);
    /*
    Json::Value root;

    for (auto& kv : paramsB) {
        root[kv.first] =  kv.second;
    }

    std::fstream file (INDEXER_BASE_SAVE_PATH + basePath+ ".json", std::fstream::in | std::fstream::out);

    file << root;
    file.close();
    */
	return true;
}

bool SRIndexer::load(string basePath){

    loadLabels(basePath);
    createNewLNReconstructor(paramsB);

    fmat empty_dictionary;
    indexKSVD = indexk_Ptr( new indexk(*lnMinQuery, empty_dictionary));
    indexKSVD->load(INDEXER_BASE_SAVE_PATH + basePath);

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

int SRIndexer::addToIndexLive(arma::fmat& features){
    features = features.t();
    std::shared_ptr<arma::fmat> featuresPtr(new arma::fmat(features));
    indexKSVD->addToIndex(featuresPtr);
    return -1;
}
