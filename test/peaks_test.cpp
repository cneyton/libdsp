#include <iostream>
#include "peaks.h"

int main()
{
    arma::vec in  = arma::regspace(0, 9) + 1;
    in.print();
    arma::uvec out = peaks::local_peaks(in, 0);
    out.print();
    return 0;
}
