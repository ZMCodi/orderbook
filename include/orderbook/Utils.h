#pragma once

#include <chrono>

using time_ = std::chrono::time_point<std::chrono::system_clock>;

namespace utils
{
    double trunc(double price, double tickSize);
    time_ now();
}
