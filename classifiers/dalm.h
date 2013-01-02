#ifndef _DALM_H_
#define _DALM_H_

#include <cassert>
#include <cmath>
#include <iostream>

#include <armadillo>

namespace l1min {

using namespace arma;

struct DALMOptions {
    uword max_iters; // 10000
    double tolerance; //1e-3
    double lambda;
    double beta;
};

class DALM {
    
private:
    
    template <class T>
    T sign(const T& matrix) const {
        typedef typename T::elem_type elem_type;
        
        T result = zeros<T>(matrix.n_rows, matrix.n_cols);
        const elem_type* matrix_elem = matrix.memptr();
        elem_type* result_elem = result.memptr();

        for (uword i = 0; i < matrix.n_elem; ++i) {
            result_elem[i] = matrix_elem[i] > 0 ? 1 : matrix_elem[i] < 0 ? -1 : 0;
        }

        return result;
    }
    
    template <class T>
    Col<typename T::elem_type> min(const T& matrix, typename T::elem_type min) const {
        typedef typename T::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        
        sign_type result = matrix;
        elem_type* matrix_elem = result.memptr();

        for (uword i = 0; i < result.n_elem; ++i) {
            matrix_elem[i] = std::min(matrix_elem[i], min);
        }
        return result;
    }
    
public:
    
    typedef DALMOptions option_type;
    
    option_type options;

    DALM(option_type opts) :
        options(opts)
    {}
    
    const std::string name() {
        return "DALM";
    }
     
    // This way the function can accept views efficiently.
    template <class T1, class T2>
    Col<typename T1::elem_type> operator()(const T1& D, const T2& x) const {

        typedef typename T1::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        typedef Mat<elem_type> dict_type;

        uword m = D.n_rows;
        uword n = D.n_cols;
        
        elem_type beta = options.beta;
        elem_type beta_inv = 1 / beta;
        
        dict_type G = D * D.t() + eye<dict_type>(m, m) * options.lambda / beta;
        
        dict_type invG = inv(G);
        dict_type A_invG_b = D.t() * (invG * x);
        
        sign_type result = zeros<sign_type>(n);
        
        sign_type z;
        sign_type result_old;
        sign_type temp1;
        sign_type temp = zeros<sign_type>(n);
        // elem_type f = 0;
        
        // bool converged = false;
        uword n_iters = 0;
        
        
        while (n_iters < options.max_iters) {
            
            n_iters++;
            
            result_old = result;
            
            // Fix x and y (temp1) and mizimize for z while keeping the restrictions in the dual.
            temp1 = temp + result * beta_inv;
            
            // std::cout << "ntemp1 " << norm(temp1, 2) << std::endl;
            
            z = sign(temp1) % min(arma::abs(temp1), 1.0);
            // if (n_iters == 2) std::cout << z << std::endl;
            // std::cout << "nz " << norm(z, 2) << std::endl;
            
            // Fix z and x and minimize for y.
            temp = D.t() * (invG * (D * (z - result * beta_inv))) + A_invG_b * beta_inv;
            
            // std::cout << "ntemp " << norm(temp, 2) << std::endl;
            
            
            // Minimize for x
            result = result - beta * (z - temp);
            
            // std::cout << "nx " << norm(result, 2) << std::endl;
            
            // Check if the result varied enough.
            // converged = norm(result_old - result, 2) < options.tolerance * norm(result_old, 2);
        }
        
        return result;
    }
    
};
}


#endif /* end of include guard: _DALM_H_ */
