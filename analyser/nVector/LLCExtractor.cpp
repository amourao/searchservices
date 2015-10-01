#include "LLCExtractor.h"

static LLCExtractor LLCExtractorFactory;

LLCExtractor::LLCExtractor(){
	FactoryAnalyser::getInstance()->registerType("LLCExtractor",this);
}

LLCExtractor::LLCExtractor(string& _type, map<string, string>& params){
	type = _type;

    if (params.size() == 0)
        return;

    if(params.count("dictPath") == 0){
        B = arma::randu<arma::mat>(std::stoi(params["dictSize"]),std::stoi(params["dimensionality"]));
        utils::normalize_columns(B);
    } else {
        B.load(params["dictPath"]);
    }

    knn = std::stoi(params["knn"]);
	beta = std::stod(params["beta"]);

    Bt = trans(B);

    BB = sum(B % B, 1);
    BBt = trans(BB);

    II = eye<mat>(knn, knn);


}

void* LLCExtractor::createType(string& type){
	if (type == "LLCExtractor")
		return new LLCExtractor();
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* LLCExtractor::createType(string& createType, map<string ,string>& params){
	return new LLCExtractor(type,params);
}

LLCExtractor::~LLCExtractor(){

}

void LLCExtractor::extractFeatures(arma::fmat& X, arma::fmat& dst){
    arma::mat Xd = conv_to<mat>::from(X);
    arma::mat dstd;
    extractFeatures(Xd,dstd);
    dst = conv_to<fmat>::from(dstd);
}

void LLCExtractor::extractFeatures(arma::mat& X, arma::mat& dst){


    int nframe = X.n_rows;
    int nbase = B.n_rows;

    arma::mat XX = sum(X % X, 1);

    arma::mat D = repmat(XX, 1, nbase) - 2*X*Bt+repmat(BBt,nframe,1);


    arma::umat IDX = zeros<umat>(knn,nframe);

    for (int i = 0; i < nframe; i++){
        arma::mat d = D.row(i);
        uvec idx = sort_index(d.t());
        IDX.col(i) = idx(span(0,knn-1));
    }
    //cout << "idx: " << IDX.rows(0,4) << endl;

    dst.set_size(nframe, nbase);
    dst.fill(0);

    for (int i = 0; i < nframe; i++){
        arma::uvec idx = IDX.col(i);
        arma::mat z = B.rows(idx) - repmat(X.row(i),knn,1);     // shift ith pt to origin
        arma::mat C = z*z.t();                                  // local covariance
        C = C + II*beta*trace(C);                               // regularlization (K>D)
        arma::mat w = solve(C,ones<mat>(knn,1));
        w = w/(accu(w));                                   // enforce sum(w)=1
        arma::mat coe = dst.row(i);
        coe.cols(idx) = w.t();
        dst.row(i) = coe;
    }
}

int LLCExtractor::getFeatureVectorSize(){
	return B.n_cols;
}

string LLCExtractor::getName(){
	return type;
}
