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

    // 'aggregate constructor' SFINAE for Trade only
    template<bool enabled = !ownsUUIDs>
    TradeImpl(const uuids::uuid* id_val, const uuids::uuid* buyer_val, const uuids::uuid* seller_val,
              double price_val, int volume_val, time_ timestamp_val, Order::Side taker_val,
              std::enable_if_t<enabled>* = nullptr)
    : id{id_val}, buyer_id{buyer_val}, seller_id{seller_val}
    , price{price_val}, volume{volume_val}
    , timestamp{timestamp_val}, taker{taker_val} {}

    // 'copy constructor' from Trade to TradeCopy
    // SFINAE for TradeCopy only
    template<bool enabled = ownsUUIDs>
    TradeImpl(const TradeImpl<false>& other, std::enable_if_t<enabled>* = nullptr)
    : id{other.id ? *other.id : uuids::uuid{}}
    , buyer_id{other.buyer_id ? *other.buyer_id : uuids::uuid{}}
    , seller_id{other.seller_id ? *other.seller_id : uuids::uuid{}}
    , price{other.price}, volume{other.volume}
    , timestamp{other.timestamp}, taker{other.taker} {}

    // Normal copy constructor (for both types)
    TradeImpl(const TradeImpl& other) = default;

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
inline void Order::notify(TradeCopy trade)
{
    if (callbackFn) {callbackFn(trade);}
}
