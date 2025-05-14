#pragma once

#include <chrono>
#include <list>
#include "orderbook/OrderBook.h"

// helper for timestamps
inline auto time_point() {
    return std::chrono::system_clock::now();
}

// helper for comparing orderlists
inline bool compareOrderLists(const std::list<Order>& first, const std::list<Order>& sec)
{
    if (first.size() != sec.size()) {return false;}

    auto f{first.begin()};
    auto s{sec.begin()};
    for (size_t i{}; i < first.size(); ++i)
    {
        if (!(*f).equals_to(*s)) {return false;}
        ++f;
        ++s;
    }

    return true;
}
