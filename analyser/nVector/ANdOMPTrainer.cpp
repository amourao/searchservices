#include "ANdOMPTrainer.h"

static ANdOMPTrainer ANdOMPTrainerFactory;

ANdOMPTrainer::ANdOMPTrainer(){
	FactoryAnalyser::getInstance()->registerType("ANdOMPTrainer",this);
}

ANdOMPTrainer::ANdOMPTrainer(string& _type, map<string, string>& params){
	type = _type;
}

ANdOMPTrainer::ANdOMPTrainer(arma::fmat& dictionary, arma::fmat& train, arma::fmat& validation, ANdOMPExtractor _fe, int _n_iters, double _eps){
    D = dictionary;
    D_seed = dictionary;
    V = validation;
    X = train;

    Gamma = arma::fmat(D.n_cols, X.n_cols);

    n_iters = _n_iters;
	fe = _fe;
	eps = _eps;
}

void* ANdOMPTrainer::createType(string& type){
	if (type == "ANdOMPTrainer")
		return new ANdOMPTrainer();
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* ANdOMPTrainer::createType(string& createType, map<string ,string>& params){
	return new ANdOMPTrainer(type,params);
}

ANdOMPTrainer::~ANdOMPTrainer(){

}

void ANdOMPTrainer::iterate(){
    G = trans(D) * D;

    for (arma::uword i = 0; i < X.n_cols; ++i) {
        arma::fmat a,b;
        a = X.col(i);
        fe.extractFeatures(a,b);
        Gamma.col(i) = b;
    }

    // std::cerr << norm(X - D * Gamma , "fro") << "\n";

    for (arma::uword j = 0; j < D.n_cols; ++j) {
        D.col(j).zeros();
        // std::cout << "Before find \n";
        const arma::uvec I_current = arma::find(arma::abs(Gamma.row(j)) > eps); // Examples using atom j.
        // std::cout << "After find \n";
        // std::cout << "Gamma cols " << Gamma.n_cols << "\n";

        // std::cout << I_current << "current\n";
        if (I_current.n_elem > 0) { // Some signal uses this atom.
           arma::uvec J(1); J(0) = j;
           arma::Col<float> g = Gamma(J, I_current).t();
           // std::cout << "After index \n";
           arma::fmat XCI = X.cols(I_current);
           arma::Col<float> d = XCI * g;
           arma::fmat GCI = Gamma.cols(I_current);
           d -= D * GCI * g;
           d /= norm(d, 2);
           g = trans(XCI) * d;
           g -= trans(D * GCI) * d;
           D.col(j) = d;
           Gamma(J, I_current) = trans(g);
        }
    }
}

int ANdOMPTrainer::getFeatureVectorSize(){
	return D.n_cols;
}

string ANdOMPTrainer::getName(){
	return type;
}
