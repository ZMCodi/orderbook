#pragma once

#include <list>
#include <map>
#include <unordered_set>

#include "Trade.h"
#include "Order.h"
#include "OrderResult.h"

// uuid generators
inline static std::mt19937 engine{Random::generate()};
inline static uuids::uuid_random_generator uuid_generator(engine);

// linked list of orders where the tail is the newest order
using order_list = std::list<Order>;

// this would ideally be a container that flushes to a database
// these are used for bookkeeping
using trade_list = std::vector<Trade>;

// persists the uuid instances for Trade and Order to point to
using id_pool = std::unordered_set<uuids::uuid>;

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

// id map = {&id: (price, &order, BUY/SELL)}
using id_map = std::unordered_map<const uuids::uuid*, OrderLocation>;

// stores state for testing
struct OrderBookState
{
    bid_map bidMap{};
    ask_map askMap{};
    id_map idMap{};
    trade_list tradeList{};

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
    OrderBook(float tickSize) : tickSize(tickSize) {}
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook(OrderBook&&) = delete;
    OrderBook& operator=(OrderBook&&) = delete;

    OrderResult placeOrder(Order& order);
    OrderResult placeOrder(Order& order, callback callbackFn);
    OrderResult placeOrder(Order&& order);
    OrderResult placeOrder(Order&& order, callback callbackFn);

    OrderResult cancelOrder(const uuids::uuid* id);
    OrderResult modifyOrder(const uuids::uuid* id, int volume, float price);
    OrderResult modifyVolume(const uuids::uuid* id, int volume);
    OrderResult modifyPrice(const uuids::uuid* id, float price);

    bool registerCallback(const uuids::uuid* id, callback callbackFn);
    bool removeCallback(const uuids::uuid* id);

    const order_list& bidsAt(float priceLevel);
    const order_list& asksAt(float priceLevel);
    int volumeAt(float priceLevel);

    const Order& getOrderByID(const uuids::uuid* id);
    const Order& getOrderByID(const uuids::uuid& id);

    Depth getDepth(size_t levels);
    Depth getDepthAtPrice(float price, size_t levels);
    Depth getDepthInRange(float maxPrice, float minPrice);

    float getBestBid() {return bestBid;}
    float getBestAsk() {return bestAsk;}
    float getMarketPrice() {return marketPrice;}
    int getTotalVolume() {return totalVolume;}
    float getSpread() {return bestAsk - bestBid;}

    // helpers for testing
    friend bool checkOBState(const OrderBook& ob, const OrderBookState& state); // for testing
    OrderBookState getState();
    id_pool getIDPool() {return idPool;}
    void clear();

    std::list<Order> dummy{{Order::Side::BUY, 3, Order::Type::LIMIT, 50}};

private:
    bid_map bidMap{};
    ask_map askMap{};
    id_map idMap{};
    trade_list tradeList{};
    id_pool idPool{};

    float bestBid{-1};
    float bestAsk{-1};
    float marketPrice{-1};
    int totalVolume{};

    const float tickSize{0.01f};
};
