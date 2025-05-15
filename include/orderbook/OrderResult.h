#pragma once

#include "Trade.h"
#include "Order.h"

using trades = std::vector<Trade>;

// helper for comparing Trades
bool compareTrades(const trades& first, const trades& second);

// order results should store copies
// and are fully managed and owned by the user
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

    uuids::uuid order_id;
    Status status;
    trades trades;
    const Order* remainingOrder;
    std::string message;

    bool operator==(const OrderResult& other) const;
    bool equals_to(const OrderResult& other) const;
};
