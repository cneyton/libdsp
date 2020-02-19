#include <iostream>
#include "correlate.h"

int main()
{
    arma::vec in  = arma::regspace(0, 9);
    arma::vec out_none     = correlate::xcorr(in);
    arma::vec out_unbiased = correlate::xcorr(in, correlate::scale::unbiased);
    arma::vec out_biased   = correlate::xcorr(in, correlate::scale::biased);
    std::cout << "none" << std::endl;
    out_none.print();
    std::cout << "unbiased" << std::endl;
    out_unbiased.print();
    std::cout << "biased" << std::endl;
    out_biased.print();
    return 0;
}
