#ifndef CORRELATE_H
#define CORRELATE_H

#include <armadillo>

class correlate
{
public:
    enum class scale {
        none,
        unbiased,
        biased
    };

    template<typename T>
    static
    arma::Col<T> xcorr(const arma::Col<T>& x, scale s = scale::none)
    {
        const arma::uword n_elem     = x.n_elem;
        const arma::uword out_n_elem = 2 * n_elem - 1;

        // zero padded version of x
        arma::Col<T> xx(n_elem + out_n_elem, arma::fill::zeros);

        const T *  x_mem =  x.memptr();
              T * xx_mem = xx.memptr();

        arma::arrayops::copy(&(xx_mem[n_elem - 1]), x_mem, n_elem);

        arma::Col<T> out(out_n_elem);
        T * out_mem = out.memptr();
        for (arma::uword i = 0; i < out_n_elem; ++i) {
            out_mem[i] = arma::op_dot::direct_dot(n_elem, x_mem, &(xx_mem[i]) );
        }

        arma::Col<T> norm;
        switch (s) {
        case scale::none:
                return out;
        case scale::unbiased:
            norm = n_elem
                 - arma::abs(arma::regspace<arma::Col<T>>(0, out_n_elem - 1) - (n_elem - 1));
            break;
        case scale::biased:
            norm = n_elem * arma::ones<arma::Col<T>>(out_n_elem);
            break;
        }

        return out/norm;
    }
};

#endif /* CORRELATE_H */
