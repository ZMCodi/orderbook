#pragma once

#include <chrono>
#include "libraries/Random.h"
#include "libraries/uuid.h"

using time_ = std::chrono::time_point<std::chrono::system_clock>;
using tick_t = int32_t;

namespace utils
{
    // uuid generators
    static std::mt19937 engine{Random::generate()};
    static uuids::uuid_random_generator uuid_generator(engine);

    double trunc(double price, double tickSize);
    time_ now();
    tick_t convertTick(double price, double tickSize);
}
