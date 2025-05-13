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
    std::list<Order> bids;
    std::list<Order> sells;
};

class OrderBook
{
    using id_map = std::unordered_map<std::string, std::pair<float, std::list<Order>::iterator>>;
    using pricelevel_map = std::map<float, PriceLevel>;
public:
    OrderBook() = default;

    OrderResult place_order(Order& order)
    {
        return {order.get_id(), OrderResult::FILLED, std::vector<Trade>(), &order, ""};
    }

    const std::list<Order>& bidsAt(float priceLevel)
    {
        [[maybe_unused]] float lol = priceLevel * 2;
        return dummy;
    }

    const std::list<Order>& asksAt(float priceLevel)
    {
        [[maybe_unused]] float lol = priceLevel * 2;
        return dummy;
    }

    const Order& getOrderByID(std::string_view id)
    {
        [[maybe_unused]] std::size_t lol = id.size();
        return *dummy.begin();
    }

    int volumeAt(float priceLevel)
    {
        [[maybe_unused]] float lol = priceLevel * 2;
        return -1;
    }

    float getBestBid() {return bestBid;}
    float getBestAsk() {return bestAsk;}
    float getMarketPrice() {return marketPrice;}
    int getTotalVolume() {return totalVolume;}

    std::list<Order> dummy{{Order::Side::BUY, 3, Order::Type::LIMIT, 50}};

private:
    pricelevel_map priceLevels{};
    id_map orderIDs{};

    float bestBid{-1};
    float bestAsk{-1};
    float marketPrice{};
    int totalVolume{};
};
