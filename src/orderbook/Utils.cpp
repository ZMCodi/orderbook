#include <cmath>
#include "orderbook/Utils.h"

time_ utils::now() {return std::chrono::high_resolution_clock::now();}

double utils::trunc(double price, double tickSize)
{
    return std::floor(price / tickSize + tickSize) * tickSize;
}

tick_t utils::convertTick(double price, double tickSize)
{
    return static_cast<tick_t>(std::trunc(price / tickSize + tickSize));
}
