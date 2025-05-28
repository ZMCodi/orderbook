#pragma once

#include <chrono>
#include "libraries/Random.h"
#include "libraries/uuid.h"

using time_ = std::chrono::time_point<std::chrono::high_resolution_clock>;
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

inline std::ostream& operator<<(std::ostream& os, const time_& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + 
        std::chrono::duration_cast<std::chrono::system_clock::duration>(
            tp.time_since_epoch()));
    return os << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
}
