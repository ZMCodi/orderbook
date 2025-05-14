#pragma once

#include "Order.h"

struct Trade
{
public:
    const uuids::uuid* id;
    const uuids::uuid* buyer_id;
    const uuids::uuid* seller_id;
    float price;
    int volume;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    Order::Side taker;

    bool equals_to(const Trade& other) const; // for testing
    bool operator==(const Trade& other) const;
    const uuids::uuid* get_id();
};
