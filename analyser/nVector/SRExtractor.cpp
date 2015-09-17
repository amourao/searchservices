#include "SRExtractor.h"

static SRExtractor srExtractorFactory;

SRExtractor::SRExtractor(){
	FactoryAnalyser::getInstance()->registerType("srExtractor",this);
}

SRExtractor::SRExtractor(string& _type, map<string, string>& params){
	type = _type;

    if (params.size() == 0)
        return;

    if(params.count("dictPath") == 0){
        dict = arma::randu<arma::fmat>(std::stoi(params["max_iters"]),std::stoi(params["dictSize"]));
        utils::normalize_columns(dict);
    } else {
        dict.load(params["dictPath"]);
    }


    l1min::OMPSparseConstrained::option_type opt;
    opt.max_iters = std::stoi(params["max_iters"]);
	opt.eps = std::stod(params["eps"]);
	omp = new l1min::OMPSparseConstrained(opt);
}

void* SRExtractor::createType(string& type){
	if (type == "SRExtractor")
		return new SRExtractor();
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* SRExtractor::createType(string& createType, map<string ,string>& params){
	return new SRExtractor(type,params);
}

SRExtractor::~SRExtractor(){
    delete omp;
}

void SRExtractor::extractFeatures(cv::Mat& src, cv::Mat& dst){
	arma::fmat srcFMat, dstFMat;
	cv::Mat dstTmp;
	MatrixTools::matToFMat(src,srcFMat);
	extractFeatures(srcFMat,dstFMat);
	MatrixTools::fmatToMat(dstFMat,dstTmp);
	dstTmp.copyTo(dst);
}

void SRExtractor::extractFeatures(arma::fmat& src, arma::fmat& dst){
	dst = (*omp)(dict,src);
	dst = trans(dst);
}

int SRExtractor::getFeatureVectorSize(){
	return dict.n_cols;
}

string SRExtractor::getName(){
	return type;
}
