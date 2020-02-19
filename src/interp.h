#ifndef INTERP_H
#define INTERP_H

#include <armadillo>
#include <stdexcept>

class interp
{
public:

template<typename T>
static
std::pair<T, T> vertex(const arma::Col<T>& y, const arma::uword x)
{
    if (y.n_elem < 3)
        throw std::logic_error("y size must be >= 3");
    if (x == 0 || x == y.n_elem -1)
        throw std::logic_error("x can't be at y boundaries");

    T y0  = y[x];
    T ym1 = y[x - 1];
    T y1  = y[x + 1];

    T a = 0.5 * (ym1 + y1 - 2*y0);
    T b = 0.5 * (y1 - ym1);
    T c = y0;

    if (a == 0.0)
        return std::pair<T, T>(static_cast<T>(x), y[x]);

    T xx = static_cast<T>(x) - b/(2*a);
    T yy = -b*b/(4*a) + c;
    return std::pair<T, T>(xx, yy);
}

};

#endif /* INTERP_H */
