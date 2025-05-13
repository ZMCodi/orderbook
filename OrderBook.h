#pragma once

#include "Trade.h"
#include "Order.h"
#include "OrderResult.h"

#include <list>
#include <map>

class OrderBook
{
    // linked list of orders, easy to swap out implementation by just changing it here
    using order_list = std::list<Order>;

    // aggregrates the volume at a given price level so no need to recalculate
    // when getting depth
    struct PriceLevel
    {
        int volume{};
        order_list orders;
    };

    // bid/ask map = {price: (volume, [orders...])}
    using bid_map = std::map<float, PriceLevel, std::greater<float>>;
    using ask_map = std::map<float, PriceLevel>;

    struct OrderLocation
    {
        float price;
        order_list::iterator itr;
        Order::Side side; // easier to determine which map to search
    };

    // id map = {id: (price, &order, BUY/SELL)}
    using id_map = std::unordered_map<std::string, OrderLocation>;
public:

    // snapshot at each price level
    struct Level
    {
        float price;
        int volume;
        int orderCount;
    };

    // snapshot of orderbook and each pricelevel
    struct Depth
    {
        std::vector<Level> bids;
        std::vector<Level> asks;
        int volume;
        float bestBid;
        float bestAsk;
        float marketPrice;
    };

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
    bid_map bidMap{};
    ask_map askMap{};
    id_map orderIDs{};

    float bestBid{-1};
    float bestAsk{-1};
    float marketPrice{};
    int totalVolume{};
};
