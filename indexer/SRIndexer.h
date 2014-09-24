#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>

#include <json/json.h>

#include <ksvd-indexing-experiment/ksvd_index_mp.h>
#include <ksvd/ksvd.h>
#include <utils/utils.h>

#include <l1min/dalm.h>
#include <l1min/fista.h>
#include <l1min/omp.h>
#include <l1min/thresholding.h>

#include <l1min/dalm_io.h>
#include <l1min/fista_io.h>
#include <l1min/omp_io.h>
#include <l1min/thresholding_io.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/flann/flann.hpp>


#include "IIndexer.h"
#include "FactoryIndexer.h"

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

	std::pair<vector<float>,vector<float> > knnSearchId(arma::fmat& name, int n);
	std::pair<vector<float>,vector<float> > radiusSearchId(arma::fmat& name, double radius, int n);

	bool save(string basePath);
	bool load(string basePath);

	int addToIndexLive(arma::fmat& features);

	string getName();

private:

    typedef l1min::OMPSparseConstrained lnmin;
    typedef l1min::OMPBatchSparseConstrained lnminK;
    typedef std::shared_ptr<lnmin> lnmin_Ptr;
    typedef std::shared_ptr<lnminK> lnminK_Ptr;

    typedef sparse::KSVDBatch<OMPBatchSparseConstrained, float> ksvdb;
    typedef std::shared_ptr<ksvdb> ksvdb_Ptr;

    typedef forr::KSVDIndex<OMPSparseConstrained, fvec> indexk;
    typedef std::shared_ptr<indexk> indexk_Ptr;


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
    double search_limit;

    arma::fmat dictionary;
    std::shared_ptr<arma::fmat> indexData;
	string type;
};
