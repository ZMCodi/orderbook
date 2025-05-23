#pragma once

#include <chrono>

using time_ = std::chrono::time_point<std::chrono::system_clock>;

namespace utils
{
    float trunc(float price, float tickSize);
    time_ now();
}
