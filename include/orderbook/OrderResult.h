#pragma once

#include "Trade.h"
#include "Order.h"

using trade_ptrs = std::vector<const Trade*>;

// helper for comparing Trades
bool compareTrades(const trade_ptrs& first, const trade_ptrs& second);

struct OrderResult{
    enum Status
    {
        PLACED,
        FILLED,
        PARTIALLY_FILLED,
        REJECTED,
        MODIFIED,
        CANCELLED,
    };

    const uuids::uuid* order_id;
    Status status;
    trade_ptrs trades;
    Order* remainingOrder;
    std::string message;

    bool operator==(const OrderResult& other) const;
    bool equals_to(const OrderResult& other) const;
};
