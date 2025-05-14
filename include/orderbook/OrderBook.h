#pragma once

#include "Trade.h"
#include "Order.h"
#include "OrderResult.h"

#include <list>
#include <map>

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
    order_list::iterator itr; // O(1) access to cancel/modify/fetch
    Order::Side side; // easier to determine which map to search
};

// id map = {id: (price, &order, BUY/SELL)}
using id_map = std::unordered_map<std::string, OrderLocation>;

// stores state for testing
struct OrderBookState
{
    bid_map bidMap{};
    ask_map askMap{};
    id_map idMap{};

    float bestBid{-1};
    float bestAsk{-1};
    float marketPrice{-1};
    int totalVolume{};
};

class OrderBook
{
public:

    // snapshot at each price level
    struct Level
    {
        float price;
        int volume;
        int orderCount;

        bool operator==(const Level& other) const;
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

        bool operator==(const Depth& other) const;
    };

    OrderBook() = default;

    OrderResult place_order(Order& order);
    const std::list<Order>& bidsAt(float priceLevel);
    const std::list<Order>& asksAt(float priceLevel);
    const Order& getOrderByID(std::string_view id);
    int volumeAt(float priceLevel);
    Depth getDepth(size_t levels);
    Depth getDepthAtPrice(float price, size_t levels);
    Depth getDepthInRange(float maxPrice, float minPrice);
    float getBestBid() {return bestBid;}
    float getBestAsk() {return bestAsk;}
    float getMarketPrice() {return marketPrice;}
    int getTotalVolume() {return totalVolume;}
    float getSpread() {return bestAsk - bestBid;}
    friend bool checkOBState(const OrderBook& ob, const OrderBookState& state); // for testing

    std::list<Order> dummy{{Order::Side::BUY, 3, Order::Type::LIMIT, 50}};

private:
    bid_map bidMap{};
    ask_map askMap{};
    id_map idMap{};

    float bestBid{-1};
    float bestAsk{-1};
    float marketPrice{-1};
    int totalVolume{};
};
