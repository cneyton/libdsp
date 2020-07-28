#include <iostream>
#include "dsp/correlate.h"

int main()
{
    arma::vec in  = arma::regspace(0, 9);
    arma::vec out_none     = correlate::xcorr(in);
    arma::vec out_unbiased = correlate::xcorr(in, correlate::scale::unbiased);
    arma::vec out_biased   = correlate::xcorr(in, correlate::scale::biased);
    return 0;
}
