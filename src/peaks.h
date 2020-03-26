#ifndef PEAKS_H
#define PEAKS_H

#include <stdexcept>
#include <vector>
#include <armadillo>

class peaks
{
public:

template<typename T>
static
arma::uvec local_peaks(const arma::Col<T>& x, const arma::uword radius)
{
    if (x.n_elem < radius) {
        std::ostringstream os;
        os << "vec size < radius : " << x.n_elem << " < " << radius;
        throw std::out_of_range(os.str());
    }

    std::vector<arma::uword> peak_inds;
    peak_inds.reserve(x.n_elem);

    if (radius == 0)
        return arma::uvec(peak_inds);

    if (x.min() == x.max())
        return arma::uvec(peak_inds);

    arma::uword i = 0;
    while (i < radius + 1) {
        if (i == arma::index_max(x.subvec(0, i + radius))) {
            peak_inds.push_back(i);
            i += radius + 1;
        } else {
            i++;
        }
    }

    while (i < x.n_elem - radius) {
        if (radius == arma::index_max(x.subvec(i - radius, i + radius))) {
            peak_inds.push_back(i);
            i += radius + 1;
        } else {
            i++;
        }
    }

    while (i < x.n_elem) {
        if (radius == arma::index_max(x.subvec(i - radius, x.n_elem - 1))) {
            peak_inds.push_back(i);
            i += radius + 1;
        } else {
            i++;
        }
    }

    return arma::uvec(peak_inds);
}

};

#endif /* PEAKS_H */
