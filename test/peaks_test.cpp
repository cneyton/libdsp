#include <iostream>
#include "dsp/peaks.h"

int main()
{
    arma::vec in   = arma::regspace(0, 9) + 1;
    arma::uvec out = peaks::local_peaks(in, 0);
    return 0;
}
