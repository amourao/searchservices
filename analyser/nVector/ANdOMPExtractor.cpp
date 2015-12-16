#include "ANdOMPExtractor.h"

static ANdOMPExtractor ANdOMPExtractorFactory;

ANdOMPExtractor::ANdOMPExtractor(){
	FactoryAnalyser::getInstance()->registerType("ANdOMPExtractor",this);
}

ANdOMPExtractor::ANdOMPExtractor(string& _type, map<string, string>& params){
	type = _type;

    if (params.size() == 0)
        return;

    if(params.count("dictPath") == 0){
        D = arma::randu<arma::fmat>(std::stoi(params["dimensionality"]),std::stoi(params["dictSize"]));
        utils::normalize_columns(D);
    } else {
        D.load(params["dictPath"]);
    }

    max_iters = std::stoi(params["max_iters"]);
	eps = std::stod(params["eps"]);
}

ANdOMPExtractor::ANdOMPExtractor(arma::fmat& dictionary, int _max_iters, double _eps){
	type = "SR";

    D = dictionary;

    max_iters = _max_iters;
	eps = _eps;
}

void* ANdOMPExtractor::createType(string& type){
	if (type == "ANdOMPExtractor")
		return new ANdOMPExtractor();
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* ANdOMPExtractor::createType(string& createType, map<string ,string>& params){
	return new ANdOMPExtractor(type,params);
}

ANdOMPExtractor::~ANdOMPExtractor(){

}

void ANdOMPExtractor::extractFeaturesAlt(arma::fmat& src, arma::fmat& dst){
	// std::cout << "Sparse Constrained" << std::endl;
    arma::Col<float> x = src;
    // Vector with the indexes of the selected atoms. A view is used to simulate the addition of indices.
    arma::uvec I = arma::zeros<arma::uvec>(max_iters);
    // The indexes in the current step.
    arma::uvec I_current;
    // Holds the Cholesky decomposition of part of the pseudo inverse.
    // A view is used to simulate the addition of rows and collumns in the incremental constructions step.
    arma::fmat L = arma::zeros<arma::fmat>(max_iters, max_iters);

    // The decomposition in the current step.
    arma::fmat L_current = arma::eye<arma::fmat>(1, 1);
    // Vector for the incremental Cholesky decomposition.
    arma::Col<float> w;
    // Residue vector.
    arma::Col<float> r = x;
    // The result.
    arma::Col<float> ipsilon = arma::zeros<arma::Col<float>>(D.n_cols);
    // Temporary to hold the correlation values.
    arma::Col<float> corr;
    // Correlations with the original signal (part of the pseudo-inverse).
    arma::Col<float> alpha = D.t() * x;
    // Number of iterations.
    uint n_iter = 0;

    // std::cout << "D begin " << D << "D end\n";

    while (n_iter < max_iters) {
        corr = abs(D.t() * r);
        arma::uword k;
        corr.max(k);

        if (corr(k) <= eps) {
            break;
        };

        // std::cout << "selecting " << k << " with value " << corr(k) << "\n";
        // std::cout << "corr " << corr << std::endl;
        // std::cout << "corr nelems " << corr.n_elem << std::endl;
        // std::cout << "r begin " << r.n_elem << "\n" << r << "r end" << std::endl;



        // std::cout << "eps " << eps << "\n";

        // std::cout << "k is " << k << "\n";

        if (n_iter == 0) {
            float a = as_scalar(trans(D.col(k)) * D.col(k));
            // std::cout << "After a\n";
            L_current(0,0) = L(0, 0) = std::sqrt(std::abs(a));
        } else {
            w = solve(trimatl(L.submat(0, 0, n_iter - 1, n_iter - 1)), trans(D.cols(I_current)) * D.col(k));
            // The new column is 0.
            // Calculate new row.
            L(n_iter, arma::span(0, n_iter - 1)) = w.t();
            // Calculate corner

            double b = as_scalar(trans(D.col(k)) * D.col(k));
            // std::cout << "After b\n";

            double c = as_scalar(w.t() * w);
            // std::cout << "After c\n";

            const float corner = std::sqrt(std::abs(b - c));
            // If the corner is close to zero we are adding a vector that is a linear combination of the previous ones.
            if (corner <= eps) {
                break;
            }

            L(n_iter, n_iter) = corner;
            L_current = L.submat(0, 0, n_iter, n_iter);
        }

        // std::cout << "L Current " << std::endl << L_current << std::endl;

        // Add k to selected atoms.
        I(n_iter) = k;
        I_current = I.subvec(0, n_iter);

        // std::cout << "at iteration " << n_iter << " I is " << I_current << std::endl;


        // Solve LL^T c = alpha_I using the Cholesky decomposition
        arma::Col<float> y = solve(trimatl(L_current), alpha.elem(I_current));
        ipsilon.elem(I_current) = solve(trimatu(L_current.t()), y);

        // Update residual

        arma::Col<float> old_r = r;

        r = x - D.cols(I_current) * ipsilon.elem(I_current);

        // std::cout << "Norm diff " << norm(r - old_r, 2) << std::endl;

        n_iter++;
    }

    dst = ipsilon.t();
}

void ANdOMPExtractor::extractFeatures(arma::fmat& src, arma::fmat& dst){
    arma::fmat G = trans(D) * D;
	// std::cout << "Sparse Constrained" << std::endl;
    arma::Col<float> x = src;
    // Vector with the indexes of the selected atoms. A view is used to simulate the addition of indices.
    arma::uvec I = arma::zeros<arma::uvec>(max_iters);
    // The indexes in the current step.
    arma::uvec I_current;
    // Holds the Cholesky decomposition of part of the pseudo inverse.
    // A view is used to simulate the addition of rows and collumns in the incremental constructions step.
    arma::fmat L = arma::zeros<arma::fmat>(max_iters, max_iters);
    L(0,0) = 1;

    // The decomposition in the current step.
    arma::fmat L_current = arma::eye<arma::fmat>(1, 1);
    // Vector for the incremental Cholesky decomposition.
    arma::Col<float> w;

    // The result.
    arma::Col<float> ipsilon = arma::zeros<arma::Col<float>>(D.n_cols);
    // Temporary to hold the correlation values.
    arma::Col<float> corr;
    // Correlations with the original signal (part of the pseudo-inverse).

    arma::Col<float> alpha0 = D.t() * x;
    arma::Col<float> alpha = arma::abs(alpha0);
    arma::Col<float> abs_alpha = alpha0;
    // Number of iterations.
    uint n_iter = 0;

    arma::uvec k_tem(1);

    // std::cout << "D begin " << D << "D end\n";

    while (n_iter < max_iters) {

        arma::uword k;
        alpha.max(k);

        if (alpha(k) <= eps) {
            break;
        };

        if (n_iter > 0) {
            k_tem(0) = k;
            w = solve(trimatl(L.submat(0, 0, n_iter - 1, n_iter - 1)), G(I_current, k_tem));
            // Calculate new row.
            L(n_iter, arma::span(0, n_iter - 1)) = w.t();
            // Calculate corner
            const float corner = std::sqrt(std::abs(1 - as_scalar(w.t() * w)));

            // If the corner is close to zero we are adding a vector that is a linear combination of the previous ones.
            if (corner <= eps) {
                break;
            }

            L(n_iter, n_iter) = corner;
            L_current = L.submat(0, 0, n_iter, n_iter);
        }

        // std::cout << "L Current " << std::endl << L_current << std::endl;

        // Add k to selected atoms.
        I(n_iter) = k;
        I_current = I.subvec(0, n_iter);

        // std::cout << "at iteration " << n_iter << " I is " << I_current << std::endl;


        // Solve LL^T c = alpha_I using the Cholesky decomposition
        arma::Col<float> y = solve(trimatl(L_current), alpha0.elem(I_current));
        ipsilon.elem(I_current) = solve(trimatu(L_current.t()), y);

        alpha = abs(alpha0 - G.cols(I_current) * ipsilon.elem(I_current));

        n_iter++;
    }

    dst = ipsilon.t();
}

int ANdOMPExtractor::getFeatureVectorSize(){
	return D.n_cols;
}

string ANdOMPExtractor::getName(){
	return type;
}

void ANdOMPExtractor::changeDictionary(arma::fmat new_dictionary){
    D = new_dictionary;
}
