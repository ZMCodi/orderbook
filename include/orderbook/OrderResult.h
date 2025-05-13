#pragma once

#include "Trade.h"
#include "Order.h"

// helper for comparing Trades
bool compareTrades(const std::vector<Trade>& first, const std::vector<Trade>& second);

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

    // consider orders getting deleted after fill, how would sv persist
    std::string_view id;
    Status status;
    std::vector<Trade> trades;
    Order* remainingOrder;
    std::string message;

    bool operator==(const OrderResult& other) const;
    bool equals_to(const OrderResult& other) const;
};
