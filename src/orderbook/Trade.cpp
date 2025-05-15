#include "orderbook/Trade.h"

bool Trade::equals_to(const Trade& other) const
{
    return buyer_id == other.buyer_id
    && seller_id == other.seller_id
    && price == other.price
    && volume == other.volume
    && taker == other.taker;
}

bool Trade::operator==(const Trade& other) const
{
    return id == other.id;
}

const uuids::uuid* Trade::get_id() const
{
    return nullptr;
}
