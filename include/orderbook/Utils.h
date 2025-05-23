#pragma once

#include <chrono>

using time_ = std::chrono::time_point<std::chrono::system_clock>;
using tick_t = int32_t;

namespace utils
{
    double trunc(double price, double tickSize);
    time_ now();
    tick_t convertTick(double price, double tickSize);
}
