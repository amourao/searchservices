#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>

#include <json/json.h>

#include <ksvd-indexing-experiment/ksvd_index_mp.h>
#include <ksvd-indexing-experiment/result.h>
#include <ksvd/ksvd.h>
#include <utils/utils.h>

#include <l1min/omp.h>
#include <l1min/omp_io.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>


#include "IIndexer.h"
#include "FactoryIndexer.h"

#define SR_DEFAULT_TRAIN_DATA_SIZE 5000
#define SR_DEFAULT_SEARCH_LIMIT 0.1
#define SR_DEFAULT_BUCKET_INSPECTION "default"
#define SR_DEFAULT_BUCKET_INSPECTION_GREEDY "greedy"
#define SR_DEFAULT_BUCKET_INSPECTION_GREEDY_NONNULL "greedy_nonnull"
#define SR_DEFAULT_BUCKET_INSPECTION_RR "rr"
#define SR_DEFAULT_BUCKET_INSPECTION_WEIGH "weighted"
#define SR_DEFAULT_BUCKET_INSPECTION_WEIGH_2 "weighted_alt"
#define SR_DEFAULT_BUCKET_INSPECTION_PARTIAL "partial"

#define SR_DEFAULT_RANDU "randu"
#define SR_DEFAULT_RANDN "randn"

using namespace pugi;
using namespace arma;
using namespace l1min;
using namespace sparse;

using namespace std;

class SRIndexer: public IIndexer {

public:

	SRIndexer();
	SRIndexer(string& type);
	SRIndexer(string& type, map<string,string>& params);
	~SRIndexer();

	void* createType(string &typeId);
	void* createType(string &typeId, map<string,string>& params);

    void train(arma::fmat& featuresTrain,arma::fmat& featuresValidationI,arma::fmat& featuresValidationQ);
	void indexWithTrainedParams(arma::fmat& features);
	void index(arma::fmat& features);

	std::pair<vector<float>,vector<float> > knnSearchId(arma::fmat& name, int n, double search_limit);
	std::pair<vector<float>,vector<float> > radiusSearchId(arma::fmat& name, double radius, int n, double search_limit);

	bool save(string basePath);
	bool load(string basePath);

	int addToIndexLive(arma::fmat& features);

	string getName();

	void deployRetrievalParameters();

	vector<double> getStatistics();
	void resetStatistics();

	void getMoreStatistics(arma::fmat& query, arma::fmat& nn);

	void printMoreStatistics();

	void normalizeByCol(arma::fmat& matrix);
	void preProcessData(arma::fmat& matrix);

	string getIndexingParameters();


private:



    typedef l1min::OMPSparseConstrained lnmin;
    typedef l1min::OMPBatchSparseConstrained lnminK;
    typedef std::shared_ptr<lnmin> lnmin_Ptr;
    typedef std::shared_ptr<lnminK> lnminK_Ptr;

    typedef sparse::KSVDBatch<OMPBatchSparseConstrained, float> ksvdb;
    typedef std::shared_ptr<ksvdb> ksvdb_Ptr;

    typedef forr::KSVDIndex<OMPSparseConstrained, fvec> indexk;
    typedef std::shared_ptr<indexk> indexk_Ptr;


    double validate(ksvdb_Ptr& dict,arma::fmat& featuresValidationI,arma::fmat& featuresValidationQ, uint n);
    void createNewLNReconstructor(map<string,string>& params);

    map<string,string> jsonToDict(Json::Value root);

    lnminK_Ptr lnMinKSVD;
    lnmin_Ptr lnMinQuery;
    ksvdb_Ptr ksvd;
    indexk_Ptr indexKSVD;

    lnmin::option_type opt;
    ksvdb::option_type optKSVD;

    int dimensions;
    int n_iter;

    int max_iters;
    double eps;
    double search_limit;
    bool normalizeCols;

    int trainDataSize;

    bool debug = false;

    string index_path;
    string bucket_inspection_method;
    string dict_seed_type;

    arma::fmat dictionary;
    std::shared_ptr<arma::fmat> indexData;
	string type;
};
