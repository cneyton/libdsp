#ifndef PEAKS_H
#define PEAKS_H

#include <stdexcept>
#include <armadillo>

class peaks
{
public:

template<typename T>
static
arma::uvec local_peaks(const arma::Col<T>& x, const arma::uword radius)
{
    if (x.n_elem < radius)
        throw std::out_of_range("x size < radius");

    arma::uvec peak_inds;

    if (radius == 0)
        return peak_inds;

    arma::uword i = 0;
    while (i < radius + 1) {
        if (i == arma::index_max(x.subvec(0, i + radius))) {
            peak_inds << i;
            i += radius;
        } else {
            i += 1;
        }
    }

    while (i < x.n_elem - radius) {
        if (i == i - radius + arma::index_max(x.subvec(i - radius, i + radius))) {
            peak_inds << i;
            i += radius;
        } else {
            i += 1;
        }
    }

    while (i < x.n_elem) {
        if (i == i - radius + arma::index_max(x.subvec(i - radius, x.n_elem - 1))) {
            peak_inds << i;
            i += radius;
        } else {
            i += 1;
        }
    }

    return peak_inds;
}

};

#endif /* PEAKS_H */
