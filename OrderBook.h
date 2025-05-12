#pragma once

#include "Trade.h"
#include "Order.h"
#include "OrderResult.h"

class OrderBook
{
public:
    OrderBook() = default;

    OrderResult place_order(Order& order)
    {
        return {order.get_id(), OrderResult::FILLED, std::vector<Trade>(), &order, ""};
    }

    const Order& bidAt(float priceLevel)
    {
        [[maybe_unused]] float lol = priceLevel * 2;
        return dummy;
    }

    const Order& askAt(float priceLevel)
    {
        [[maybe_unused]] float lol = priceLevel * 2;
        return dummy;
    }

    Order dummy{Order::Side::BUY, 3, Order::Type::LIMIT, 50};

private:
    std::map<float, std::set<Order>> priceLevels;

};
