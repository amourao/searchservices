#ifndef _L1MIN_OMP_H_
#define _L1MIN_OMP_H_

#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>

#include <armadillo>

namespace l1min {

using namespace arma;

struct OMPSparseConstrainedOptions {
    uword max_iters;
    double eps;
};

class OMPSimple {

public:

    typedef OMPSparseConstrainedOptions option_type;

    option_type options;

    OMPSimple(option_type opts) :
        options(opts)
    {}

    const std::string name() {
        return "OMP";
    }

    template <class T1, class T2>
    Col<typename T1::elem_type> operator()(const T1& D, const T2& x) const {

        // std::cout << "Simple" << std::endl;

        typedef typename T1::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        //typedef Mat<elem_type> dict_type;

        assert(options.max_iters <= D.n_cols);

        const uword max_iters = options.max_iters;
        const elem_type eps = options.eps;

        uvec I = zeros<uvec>(options.max_iters);
        sign_type r = x;
        sign_type ipsilon = zeros<sign_type>(D.n_cols);

        uword niters = 0;
        while (niters < max_iters) {
            niters++;

            sign_type corr = abs(D.t() * r);
            uword k;
            corr.max(k);

            // std::cout << "selecting " << k << " with value " << corr(k) << "\n";

            // std::cout << "k is " << k << "\n";

            if (corr(k) <= eps) {
                break;
            };

            I(niters - 1) = k;
            uvec I_current = I.subvec(0, niters - 1);
            ipsilon.elem(I_current) = pinv(D.cols(I_current)) * x;

            sign_type old_r = r;

            r = x - D.cols(I_current) * ipsilon.elem(I_current);

            // std::cout << "Norm diff " << norm(r - old_r, 2) << std::endl;

        }

        // std::cout << "Norm y " << norm(ipsilon, 2) << std::endl;
        // std::cout << "Norm x " << norm(x, 2) << std::endl;
        // std::cout << norm(x - D * ipsilon, 2) << std::endl;

        return ipsilon;

    }

};

class OMPSparseConstrained {

public:

    typedef OMPSparseConstrainedOptions option_type;

    option_type options;

    OMPSparseConstrained(option_type opts) :
        options(opts)
    {}

    const std::string name() {
        return "OMP";
    }

    // This way the function can accept views efficiently.
    template <class T1, class T2>
    Col<typename T1::elem_type> operator()(const T1& D, const T2& x) const {

        // std::cout << "Sparse Constrained" << std::endl;

        typedef typename T1::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        typedef Mat<elem_type> dict_type;

        assert(options.max_iters <= D.n_cols);

        const uword max_iters = options.max_iters;
        const elem_type eps = options.eps;

        // Vector with the indexes of the selected atoms. A view is used to simulate the addition of indices.
        uvec I = zeros<uvec>(max_iters);
        // The indexes in the current step.
        uvec I_current;
        // Holds the Cholesky decomposition of part of the pseudo inverse.
        // A view is used to simulate the addition of rows and collumns in the incremental constructions step.
        dict_type L = zeros<dict_type>(max_iters, max_iters);

        // The decomposition in the current step.
        dict_type L_current = eye<dict_type>(1, 1);
        // Vector for the incremental Cholesky decomposition.
        sign_type w;
        // Residue vector.
        sign_type r = x;
        // The result.
        sign_type ipsilon = zeros<sign_type>(D.n_cols);
        // Temporary to hold the correlation values.
        sign_type corr;
        // Correlations with the original signal (part of the pseudo-inverse).
        sign_type alpha = D.t() * x;
        // Number of iterations.
        uword n_iter = 0;

        // std::cout << "D begin " << D << "D end\n";

        while (n_iter < max_iters) {
            corr = abs(D.t() * r);
            uword k;
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
                elem_type a = as_scalar(trans(D.col(k)) * D.col(k));
                // std::cout << "After a\n";
                L_current(0,0) = L(0, 0) = std::sqrt(std::abs(a));
            } else {
                w = solve(trimatl(L.submat(0, 0, n_iter - 1, n_iter - 1)), trans(D.cols(I_current)) * D.col(k));
                // The new column is 0.
                // Calculate new row.
                L(n_iter, span(0, n_iter - 1)) = w.t();
                // Calculate corner

                double b = as_scalar(trans(D.col(k)) * D.col(k));
                // std::cout << "After b\n";

                double c = as_scalar(w.t() * w);
                // std::cout << "After c\n";

                const elem_type corner = std::sqrt(std::abs(b - c));
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
            sign_type y = solve(trimatl(L_current), alpha.elem(I_current));
            ipsilon.elem(I_current) = solve(trimatu(L_current.t()), y);

            // Update residual

            sign_type old_r = r;

            r = x - D.cols(I_current) * ipsilon.elem(I_current);

            // std::cout << "Norm diff " << norm(r - old_r, 2) << std::endl;

            n_iter++;
        }

        return ipsilon;
    }

};


// Only for dictionaries with normalized columns
class OMPBatchSparseConstrained {

public:

    typedef OMPSparseConstrainedOptions option_type;

    option_type options;

    OMPBatchSparseConstrained(option_type opts) :
        options(opts)
    {}

    const std::string name() {
        return "OMP";
    }

    // This way the function can accept views efficiently.
    template <class T1, class T2, class T3>
    Col<typename T1::elem_type> operator()(const T1& D, const T3& G, const T2& x) const {

        typedef typename T1::elem_type elem_type;
        typedef Col<elem_type> sign_type;
        typedef Mat<elem_type> dict_type;

        assert(options.max_iters <= D.n_cols);
        // Irrelevant after this point the whole space is spawned.
        const uword max_iters = std::min(options.max_iters, D.n_rows);
        const elem_type eps = options.eps;

        // Vector with the indexes of the selected atoms. A view is used to simulate the addition of indices.
        uvec I = zeros<uvec>(max_iters);
        // The indexes in the current step.
        uvec I_current;
        // Holds the Cholesky decomposition of part of the pseudo inverse.
        // A view is used to simulate the addition of rows and collumns in the incremental constructions step.
        dict_type L = zeros<dict_type>(max_iters, max_iters);
        L(0,0) = 1;
        // The decomposition in the current step.
        dict_type L_current = eye<dict_type>(1, 1);
        // Vector for the incremental Cholesky decomposition.
        sign_type w;
        // The result.
        sign_type ipsilon = zeros<sign_type>(D.n_cols);
        // Temporary to hold the correlation values.
        sign_type corr;
        // Correlations with the original signal (part of the pseudo-inverse).
        sign_type alpha0 = D.t() * x;
        sign_type alpha = abs(alpha0);
        sign_type abs_alpha = alpha0;
        // Number of iterations.
        uword n_iter = 0;

        uvec k_tem(1);

        while (n_iter < max_iters) {
            uword k;

            alpha.max(k);

            if (alpha(k) <= eps) {
                break;
            };

            // std::cout << "size of alpha " << alpha.n_cols << " " << alpha.n_rows << "\n";
            // std::cout << "selecting " << k << " with value " << alpha(k) << "\n";

            // std::cout << "max corr " << k << std::endl;

            if (n_iter > 0) {
                k_tem(0) = k;
                w = solve(trimatl(L.submat(0, 0, n_iter - 1, n_iter - 1)), G(I_current, k_tem));
                // The new column is 0.
                // Calculate new row.
                L(n_iter, span(0, n_iter - 1)) = w.t();
                // Calculate corner
                const elem_type corner = std::sqrt(std::abs(1 - as_scalar(w.t() * w)));
                // If the corner is close to zero we are adding a vector that is a linear combination of the previous ones.
                if (corner <= eps) {
                    break;
                }

                L(n_iter, n_iter) = corner;
                L_current = L.submat(0, 0, n_iter, n_iter);
            }

            // Add k to selected atoms.
            I(n_iter) = k;
            I_current = I.subvec(0, n_iter);
            // Solve LL^T c = alpha_I using the Cholesky decomposition
            sign_type y = solve(trimatl(L_current), alpha0.elem(I_current));
            ipsilon.elem(I_current) = solve(trimatu(L_current.t()), y);

            alpha = abs(alpha0 - G.cols(I_current) * ipsilon.elem(I_current));

            n_iter++;
        }

        return ipsilon;
    }

};
}


#endif /* end of include guard: _L1MIN_OMP_H_ */
