#ifndef _FISTA_STEP_
#define _FISTA_STEP_

#include <armadillo>

namespace l1min {
    
using namespace arma;

struct FISTAOptions {
    uword max_iters;
    double L;
    double tolerance;
    double lambda;
};

class FISTA {

private:
    
    template <class T>
    Col<typename T::elem_type> soft_thresholding(const T& x, double lambda) const {
        
        typedef typename T::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        
        double th = lambda;
        
        sign_type result = x;
        
        for (uword i = 0; i < result.n_elem; i++) {
            if (result(i) > th) {
                result(i) -= th;
            } else if (result(i) < -th) {
                result(i) += th;
            } else {
                result(i) = 0;
            }
        }
        
        return result;
    }

public:
    
    typedef FISTAOptions option_type;
    
    option_type options;
    
    FISTA(option_type opt):
        options(opt)
    {}
    
    const std::string name() {
        return "FISTA";
    }
    
    template <class T1, class T2>
    Col<typename T1::elem_type> operator()(const T1& D, const T2& x) const {

        typedef typename T1::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        typedef Mat<elem_type> dict_type;
        
        sign_type xk   = zeros<sign_type>(D.n_cols);
        sign_type yk   = xk;
        sign_type xkm1 = xk;
        sign_type gk;
        
        double tk   = 1;
        double tkp1 = 1;
        
        uword niter = 0;
        
        // 
        // double norm_bmAmxk = norm(x - D * xk, 2);
        // double old_f = 0.5 * norm_bmAmxk * norm_bmAmxk + options.lambda * norm(xk, 1);
        // 
        while (niter < options.max_iters) {
            
            niter++;

            gk = yk - (1.0 / options.L) * trans(D) * (D * yk - x);
            
            xk = soft_thresholding(gk, options.lambda / options.L);
            
            tkp1 = (1.0 + std::sqrt(1 + 4 * tk * tk)) / 2.0;
            
            yk = xk + ((tk - 1.0) / tkp1) * (xk - xkm1);
            
            // double norm_bmAmxk2 = norm(x - D * xk, 2);
            // double new_f = 0.5 * norm_bmAmxk2 * norm_bmAmxk2 + options.lambda * norm(xk, 1);
            // 
            // if (std::abs(new_f - old_f) <= std::abs(old_f) * options.tolerance) {
            //     break;
            // }

            xkm1 = xk;
            tk = tkp1;
            // old_f = new_f;
            
        }
        
        return xk;
        
    }
    
      
    
};

}

#endif /* end of include guard: _FISTA_STEP_ */
