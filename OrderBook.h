#pragma once

#include "Trade.h"
#include "Order.h"
#include "OrderResult.h"

#include <list>
#include <map>

struct PriceLevel
{
    float price;
    int volume;
    std::list<Order> orders;
};

class OrderBook
{
    using id_map = std::unordered_map<std::string, std::pair<double, std::list<Order>::iterator>>;
    using pricelevel_map = std::map<float, PriceLevel>;
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

    float getBestBid() {return bestBid;}
    float getBestAsk() {return bestAsk;}

    Order dummy{Order::Side::BUY, 3, Order::Type::LIMIT, 50};

private:
    pricelevel_map priceLevels;
    id_map orderIDs;

    float bestBid{-1};
    float bestAsk{-1};
};
