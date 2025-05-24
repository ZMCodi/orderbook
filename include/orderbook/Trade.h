#pragma once

#include <type_traits>
#include "Order.h"

template<bool ownsUUIDs>
struct TradeImpl
{
public:
    // Trade (internally stored) would store pointers to UUID
    // TradeCopy (user copy) would store the actual UUID itself
    std::conditional_t<ownsUUIDs, uuids::uuid, const uuids::uuid* const> id;
    std::conditional_t<ownsUUIDs, uuids::uuid, const uuids::uuid* const> buyer_id;
    std::conditional_t<ownsUUIDs, uuids::uuid, const uuids::uuid* const> seller_id;
    const double price;
    const int volume;
    const time_ timestamp;
    const Order::Side taker;

    bool equals_to(const TradeImpl<ownsUUIDs>& other) const // for testing
    {
        return buyer_id == other.buyer_id
        && seller_id == other.seller_id
        && price == other.price
        && volume == other.volume
        && taker == other.taker;
    }

    bool operator==(const TradeImpl<ownsUUIDs>& other) const
    {
        return id == other.id;
    }

    std::conditional_t<ownsUUIDs, uuids::uuid, const uuids::uuid*> get_id() const
    {
        return id;
    }
};

// this has to be put here because Trade has to be defined
inline void Order::notify(Trade trade)
{
    if (callbackFn) {callbackFn(trade);}
}
