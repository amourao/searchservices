#ifndef _THREASHOLDING_H_
#define _THREASHOLDING_H_

#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>

#include <armadillo>

namespace l1min {

using namespace arma;

struct ThresholdingOptions {
    uword max_iters;
};

class Thresholding {

public:
    
    typedef ThresholdingOptions option_type;
    
    option_type options;

    Thresholding(option_type opts) :
        options(opts)
    {}
    
    const std::string name() {
        return "Thresholding";
    }
    
    // This way the function can accept views efficiently.
    template <class T1, class T2>
    Col<typename T1::elem_type> operator()(const T1& D, const T2& x) const {

        typedef typename T1::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        typedef Mat<elem_type> dict_type;
        
        assert(options.max_iters <= D.n_cols);

        // Never solve a square or underconstrained system. Baddd.
        const uword k = std::min(options.max_iters, D.n_rows - 1);
        
        // std::cout << "Real iterations " << k << "\n";

        // The result.
        sign_type ipsilon = zeros<sign_type>(D.n_cols);

        // Correlations with the original signal (part of the pseudo-inverse).
        sign_type corr = abs(D.t() * x);
        uvec indexes = sort_index(corr, 1);
        indexes = indexes.subvec(0, k - 1);
        
        // Must use the pinv since solve tries exact when m = n
        // Use solve it's faster as we check for over determined ness
        ipsilon.elem(indexes) = solve(D.cols(indexes), x);
        
        return ipsilon;
    }
    
};
}


#endif /* end of include guard: _THREASHOLDING_H_ */
