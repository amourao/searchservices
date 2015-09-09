#include "SRExtractor.h"


SRExtractor::SRExtractor(){
    dict = arma::randu<arma::fmat>(960,1024);
    utils::normalize_columns(dict);

    l1min::OMPSparseConstrained::option_type opt;
    opt.max_iters = 25;
	opt.eps = 0.05;
	omp = new l1min::OMPSparseConstrained(opt);
}

void* SRExtractor::createType(string& type){
	if (type == "SRExtractor")
		return new SRExtractor();
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

SRExtractor::~SRExtractor(){
    delete omp;
}

void SRExtractor::extractFeatures(cv::Mat& src, cv::Mat& dst){
	arma::fmat srcFMat, dstFMat;
	cv::Mat dstTmp;
	MatrixTools::matToFMat(src,srcFMat);
	cout << srcFMat.n_rows << " " << srcFMat.n_cols << endl;
	extractFeatures(srcFMat,dstFMat);
	MatrixTools::fmatToMat(dstFMat,dstTmp);
	dstTmp.copyTo(dst);
}

void SRExtractor::extractFeatures(arma::fmat& src, arma::fmat& dst){
	dst = (*omp)(dict,src);
	cout << dst.n_rows << " " << dst.n_cols << endl;
	dst = trans(dst);
}

int SRExtractor::getFeatureVectorSize(){
	return 1024;
}

string SRExtractor::getName(){
	return "SRExtractor";
}
