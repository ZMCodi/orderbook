#include <cmath>
#include "orderbook/Utils.h"

time_ utils::now() {return std::chrono::system_clock::now();}
float utils::trunc(float price, float tickSize)
{
    double result = std::floor(static_cast<double>(price) / tickSize) * tickSize;
    return static_cast<float>(result);
}
