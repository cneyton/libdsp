#include <iostream>
#include "dsp/interp.h"

int main()
{
    arma::vec in =  - arma::abs(arma::regspace(0, 9) - 5.3);
    [[gnu::unused]] auto res  = interp::vertex(in, 5);
    return 0;
}
