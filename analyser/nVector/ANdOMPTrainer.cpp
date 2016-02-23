#include "ANdOMPTrainer.h"

static ANdOMPTrainer ANdOMPTrainerFactory;

ANdOMPTrainer::ANdOMPTrainer(){
	FactoryAnalyser::getInstance()->registerType("ANdOMPTrainer",this);
}

ANdOMPTrainer::ANdOMPTrainer(string& _type, map<string, string>& params){
	type = _type;
}

ANdOMPTrainer::ANdOMPTrainer(ANdOMPExtractor _fe, int _n_iters, double _eps, uint _dimensions){

    dimensions = _dimensions;
    n_iters = _n_iters;
	fe = _fe;
	eps = _eps;
	withBias = false;

    positiveOnly = true;
    expon = 0;
    regFactor = 0;
    weight = 0;

}

ANdOMPTrainer::ANdOMPTrainer(ANdOMPExtractor _fe, int _n_iters, double _eps, uint _dimensions, double _expon, double _regFactor, double _weight, bool _withBias, bool _positiveOnly){

    dimensions = _dimensions;
    n_iters = _n_iters;
	fe = _fe;
	eps = _eps;
	withBias = _withBias;

    expon = _expon;
    regFactor = _regFactor;
    weight = _weight;

    positiveOnly = _positiveOnly;


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

void ANdOMPTrainer::train(arma::fmat& dictionary, arma::fmat& train, arma::fmat& validationI, arma::fmat& validationQ){
    D = dictionary;
    D_seed = dictionary;

    V = validationI;
    X = train;

    Gamma = arma::fmat(D.n_cols, X.n_cols);

    cout << "niter ANd: " << n_iters << endl;

    countAppearing = arma::zeros<arma::vec>(D.n_cols);
    sumAppearing   = arma::zeros<arma::vec>(D.n_cols);

    lastCountAppearing = arma::ones<arma::vec>(D.n_cols);
    lastSumAppearing   = arma::ones<arma::vec>(D.n_cols);

    for(uint i = 0; i < n_iters; i++){
        cout << "niter ANd: " << i << endl;
        iterate();
    }
}

void ANdOMPTrainer::iterate(){

    countAppearing = arma::zeros<arma::vec>(D.n_cols);
    sumAppearing   = arma::zeros<arma::vec>(D.n_cols);

    fe.changeDictionary(D);
    arma::fmat G = D.t() * D;

    for (arma::uword i = 0; i < X.n_cols; ++i) {
        arma::fmat a,b;
        a = X.col(i);
        fe.extractFeaturesBatchSparseConstrained(a,G,b);

        if(withBias){

            arma::fmat tmp = arma::conv_to<arma::fmat>::from(b.t() % (1.0/(arma::pow(lastCountAppearing,expon)+regFactor)));
            Gamma.col(i) = tmp;
        }
        else
            Gamma.col(i) = b.t();

        arma::fmat c = b;

        if (positiveOnly)
            c.elem( find(c > 0) ).ones();
        else
            c.elem( find(c != 0) ).ones();

        countAppearing +=  arma::conv_to<arma::mat>::from(c.t());
        sumAppearing += arma::conv_to<arma::mat>::from(b.t());
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

    lastCountAppearing = countAppearing;
    lastSumAppearing = sumAppearing;
}

int ANdOMPTrainer::getFeatureVectorSize(){
	return D.n_cols;
}

string ANdOMPTrainer::getName(){
	return type;
}
