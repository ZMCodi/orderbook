#include "orderbook/Trade.h"

using ptr = const uuids::uuid*;
using tp = std::chrono::time_point<std::chrono::system_clock>;
Trade::Trade(uint64_t id, ptr buyer_id, ptr seller_id, float price, int vol, tp timestamp, Order::Side taker)
: id{id}, buyer_id{buyer_id}, seller_id{seller_id}
, price{price}, volume{vol}, timestamp{timestamp}
, taker{taker} {}

bool Trade::equals_to(const Trade& other) const
{
    return id == other.id
    && buyer_id == other.buyer_id
    && seller_id == other.seller_id
    && price == other.price
    && volume == other.volume
    && taker == other.taker;
}

bool Trade::operator==(const Trade& other) const
{
    return id == other.id;
}

const uuids::uuid* Trade::get_id()
{
    return nullptr;
}
