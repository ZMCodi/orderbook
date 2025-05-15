#pragma once

#include "Order.h"

struct Trade
{
public:
    const uuids::uuid* const id;
    const uuids::uuid* const buyer_id;
    const uuids::uuid* const seller_id;
    const float price;
    const int volume;
    const std::chrono::time_point<std::chrono::system_clock> timestamp;
    const Order::Side taker;

    bool equals_to(const Trade& other) const; // for testing
    bool operator==(const Trade& other) const;
    const uuids::uuid* get_id() const;
};
