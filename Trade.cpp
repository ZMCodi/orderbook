#include "Trade.h"

bool Trade::is_equal(const Trade& other) const
{
    return buyer_id == other.buyer_id
    && seller_id == other.seller_id
    && price == other.price
    && volume == other.volume
    && agressor == other.agressor;
}

bool Trade::operator==(const Trade& other) const
{
    return id == other.id;
}
