#include <iostream>
#include "interp.h"

int main()
{
    arma::vec in =  - arma::abs(arma::regspace(0, 9) - 5.3);
    in.print();
    auto [x, y] = interp::vertex(in, 5);
    std::cout << x << "|" << y << std::endl;
    return 0;
}
