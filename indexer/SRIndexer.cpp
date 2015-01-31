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

    search_limit = 0;
    if (params.size() > 0){
        dimensions = std::stoi(params["dimensions"]);
        n_iter = std::stoi(params["n_iter"]);

        max_iters = std::stoi(params["max_iters_ksvd"]);
        eps = std::stod(params["eps_ksvd"]);
        //index_path = paramsB["index_path"];
        createNewLNReconstructor(params);
        //load(paramsB["index_path"]);

        bucket_inspection_method = SR_DEFAULT_BUCKET_INSPECTION;


        trainDataSize = SR_DEFAULT_TRAIN_DATA_SIZE;
        if(params.count("trainDataSize") > 0)
            trainDataSize = std::stoi(params["trainDataSize"]);

        search_limit = SR_DEFAULT_SEARCH_LIMIT;

        type = typeId;
        paramsB = params;
    }
}

void SRIndexer::deployRetrievalParameters(){
    map<string,string> curr = getCurrentRetreivalParameters();
    if(curr.count("search_limit") > 0)
        search_limit = std::stod(curr["search_limit"]);
    if(curr.count("bucket_inspection") > 0)
        bucket_inspection_method = curr["bucket_inspection"];
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
        //std::cout << "Iteration " << i+1 << std::endl;
        ksvd->iterate();
    }

}

void SRIndexer::indexWithTrainedParams(arma::fmat& features){
    features = features.t();

    indexData = std::make_shared<fmat>(features);

    indexKSVD = indexk_Ptr( new indexk(*lnMinQuery, ksvd->D));
    indexKSVD->load(indexData, lnMinQuery->options.max_iters);
}

void SRIndexer::index(arma::fmat& features){

    features = features.t();

    arma::fmat trainSplit;
    vector<arma::fmat> in;
    vector<arma::fmat> out;

    in.push_back(features);

    MatrixTools::getRandomSample(in,trainDataSize,out);

    trainSplit = out.at(0).t();

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

std::pair<vector<float>,vector<float> > SRIndexer::knnSearchId(arma::fmat& query, int n, double search_limit_param){

    query = query.t();

	vector<float> indices;
	vector<float> dists;

    double current_search_limit;
    if (search_limit == 0)
        current_search_limit = search_limit_param;
    else
        current_search_limit = search_limit;

    std::vector<forr::Result> ksvd_res;

    if(bucket_inspection_method == SR_DEFAULT_BUCKET_INSPECTION_GREEDY){
        ksvd_res = indexKSVD->find_k_nearest_limit_greedy(query, n, current_search_limit*indexKSVD->size());
    } else if(bucket_inspection_method == SR_DEFAULT_BUCKET_INSPECTION_RR){
        ksvd_res = indexKSVD->find_k_nearest_limit_round_robin(query, n, current_search_limit*indexKSVD->size());
    } else if(bucket_inspection_method == SR_DEFAULT_BUCKET_INSPECTION_WEIGH){
        ksvd_res = indexKSVD->find_k_nearest_limit_wcoeff(query, n, current_search_limit*indexKSVD->size());
    } else if(bucket_inspection_method == SR_DEFAULT_BUCKET_INSPECTION_WEIGH_2){
        ksvd_res = indexKSVD->find_k_nearest_limit_wcoeff2(query, n, current_search_limit*indexKSVD->size());
    } else { //use default
        ksvd_res = indexKSVD->find_k_nearest_limit(query, n, current_search_limit*indexKSVD->size());
    }

    indices.reserve(ksvd_res.size());
    dists.reserve(ksvd_res.size());
    for (forr::Result& res : ksvd_res) {
        indices.push_back(res.vector_pos);
        dists.push_back(0);
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

vector<long> SRIndexer::getStatistics(){
    vector<long> statistics;

    auto s = indexKSVD->getStatistics();

    statistics.push_back(s.t0);
    statistics.push_back(s.t1);
    statistics.push_back(s.t2);
    statistics.push_back(s.t3);
    statistics.push_back(s.t4);
    statistics.push_back(s.t5);
    statistics.push_back(s.t6);

    return statistics;
}

void SRIndexer::resetStatistics(){
    indexKSVD->resetStatistics();
}

int SRIndexer::addToIndexLive(arma::fmat& features){
    features = features.t();
    std::shared_ptr<arma::fmat> featuresPtr(new arma::fmat(features));
    indexKSVD->addToIndex(featuresPtr);

    return 0;
}

