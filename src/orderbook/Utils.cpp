#include <cmath>
#include "orderbook/Utils.h"

time_ utils::now() {return std::chrono::system_clock::now();}
double utils::trunc(double price, double tickSize)
{
    return std::floor(price / tickSize) * tickSize;
}
