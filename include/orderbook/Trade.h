#pragma once

#include "Order.h"

struct Trade
{
public:
    const uuids::uuid* const id;
    const uuids::uuid* const buyer_id;
    const uuids::uuid* const seller_id;
    const double price;
    const int volume;
    const time_ timestamp;
    const Order::Side taker;

    bool equals_to(const Trade& other) const; // for testing
    bool operator==(const Trade& other) const;
    const uuids::uuid* get_id() const;
};

// this has to be put here because Trade has to be defined
inline void Order::notify(Trade trade)
{
    if (callbackFn) {callbackFn(trade);}
}
