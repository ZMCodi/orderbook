#pragma once

#include "Order.h"

struct Trade
{
private:
    // id should be private since they are represented
    // internally as ints instead of uuid
    uint64_t id;
public:
    const uuids::uuid* buyer_id;
    const uuids::uuid* seller_id;
    float price;
    int volume;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    Order::Side taker;

    using ptr = const uuids::uuid*;
    using tp = std::chrono::time_point<std::chrono::system_clock>;
    Trade(uint64_t id, ptr buyer_id, ptr seller_id, float price, int vol, tp timestamp, Order::Side taker);
    bool equals_to(const Trade& other) const; // for testing
    bool operator==(const Trade& other) const;
    const uuids::uuid* get_id();
};
