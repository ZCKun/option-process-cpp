#include <iostream>
#include "iv.h"


int main() 
{
    double s = 3.44516;
    double k = 3.1;
    double t = 0.040323788;
    double b = .0;
    double r = .0;
    double vol = 27.65;

    auto d1 = get_d1(s, k, t, b, vol);
    auto n1 = nd(d1);
    auto gm = gamma(s, k, t, r, b, vol);

    std::cout << d1 << std::endl;
    std::cout << n1 << std::endl;
    std::cout << gm << std::endl;

    return 0;
}
