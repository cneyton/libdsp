#ifndef INTERP_H
#define INTERP_H

#include <armadillo>
#include <stdexcept>

class interp
{
public:

template<typename T>
static
std::array<T, 2> vertex(const arma::Col<T>& y, const arma::uword x)
{
    if (y.n_elem < 3)
        throw std::logic_error("y size must be >= 3");
    if (x == 0 || x >= y.n_elem - 1)
        throw std::out_of_range("x out of range or at y boundaries");

    T y0  = y.at(x);
    T ym1 = y.at(x - 1);
    T y1  = y.at(x + 1);

    T a = 0.5 * (ym1 + y1 - 2*y0);
    T b = 0.5 * (y1 - ym1);
    T c = y0;

    if (a == 0.0)
        return {static_cast<T>(x), y.at(x)};

    T xx = static_cast<T>(x) - b/(2*a);
    T yy = -b*b/(4*a) + c;
    return {xx, yy};
}

};

#endif /* INTERP_H */
