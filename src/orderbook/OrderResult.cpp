#include "orderbook/OrderResult.h"

// helper for comparing trades
bool compareTrades(const trades& first, const trades& second)
{
    if (first.size() != second.size()) {return false;}

    for (size_t i{}; i < first.size(); ++i)
    {
        if (!first[i].equals_to(second[i])) {return false;}
    }

    return true;
}

bool OrderResult::operator==(const OrderResult& other) const
{
    return order_id == other.order_id
    && status == other.status
    && remainingOrder == other.remainingOrder
    && message == other.message
    && compareTrades(trades, other.trades);
}

bool OrderResult::equals_to(const OrderResult& other) const
{
    return *this == other;
}
