#pragma once

#include "Trade.h"
#include "Order.h"

// helper for comparing Trades
bool compareTrades(const std::vector<Trade>& first, const std::vector<Trade>& second)
{
    if (first.size() != second.size()) {return false;}

    for (std::size_t i{}; i <= first.size(); ++i)
    {
        if (!first[i].is_equal(second[i])) {return false;}

    }

    return true;
}

struct OrderResult{
    enum Status
    {
        PLACED,
        FILLED,
        PARTIALLY_FILLED,
        REJECTED,
    };

    std::string_view id;
    Status status;
    std::vector<Trade> trades;
    Order* remainingOrder;
    std::string message;

    bool operator==(const OrderResult& other) const
    {
        return id == other.id
        && status == other.status
        && remainingOrder == other.remainingOrder
        && message == other.message
        && compareTrades(trades, other.trades);
    }

    bool equals_to(const OrderResult& other) const
    {
        return *this == other;
    }
};
