#pragma once

#include <list>
#include <map>
#include <unordered_set>

#include "Trade.h"
#include "Order.h"
#include "OrderResult.h"

// linked list of orders where the tail is the newest order
using order_list = std::list<Order>;

// keeps track of modified/cancelled orders
struct OrderAudit
{
    const uuids::uuid* id;
    time_ timestamp;
    int volume_delta; // positive for decrease, -1 for cancellation
    // we can skip tracking price change and volume increase
    // since they are implemented as cancellation + new order
    // hence cancellation is tracked here and new order in OrderList
    bool equals_to(const OrderAudit& other) const;
};

// these would ideally be containers that flush to a database
// these are used for bookkeeping
using trade_list = std::vector<Trade>;
using orders = std::vector<Order>;
using audit_list = std::vector<OrderAudit>;

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
using bid_map = std::map<tick_t, PriceLevel, std::greater<tick_t>>;
using ask_map = std::map<tick_t, PriceLevel>;

struct OrderLocation
{
    double price;
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
    orders orderList{};

    double bestBid{-1};
    double bestAsk{-1};
    double marketPrice{-1};
    int totalVolume{};
};

class OrderBook
{
public:

    // snapshot at each price level
    struct Level
    {
        double price;
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
        double bestBid;
        double bestAsk;
        double marketPrice;

        bool operator==(const Depth& other) const;
    };

    OrderBook() = default;
    OrderBook(double tickSize) : tickSize(tickSize) {}
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook(OrderBook&&) = delete;
    OrderBook& operator=(OrderBook&&) = delete;

    OrderResult placeOrder(Order& order, callback callbackFn = nullptr);
    OrderResult placeOrder(Order&& order, callback callbackFn = nullptr);

    OrderResult cancelOrder(const uuids::uuid* id);
    OrderResult modifyVolume(const uuids::uuid* id, int volume);
    OrderResult modifyPrice(const uuids::uuid* id, double price);

    // reference overload
    OrderResult cancelOrder(const uuids::uuid& id);
    OrderResult modifyVolume(const uuids::uuid& id, int volume);
    OrderResult modifyPrice(const uuids::uuid& id, double price);

    bool registerCallback(const uuids::uuid* id, callback callbackFn);
    bool removeCallback(const uuids::uuid* id);

    // reference overload
    bool registerCallback(const uuids::uuid& id, callback callbackFn);
    bool removeCallback(const uuids::uuid& id);

    const order_list& bidsAt(double priceLevel);
    const order_list& asksAt(double priceLevel);
    int volumeAt(double priceLevel);

    const Order& getOrderByID(const uuids::uuid* id);
    const Order& getOrderByID(const uuids::uuid& id);

    Depth getDepth(size_t levels);
    Depth getDepthAtPrice(double price, size_t levels);
    Depth getDepthInRange(double maxPrice, double minPrice);

    double getBestBid() {if (bestBid == -1) throw std::runtime_error{"No bids available"}; return bestBid;}
    double getBestAsk() {if (bestAsk == -1) throw std::runtime_error{"No asks available"}; return bestAsk;}
    double getMarketPrice() {if (marketPrice == -1) throw std::runtime_error("No trades yet"); return marketPrice;}
    int getTotalVolume() {return totalVolume;}
    double getSpread() {return getBestAsk() - getBestBid();}

    // helpers for testing
    friend bool checkOBState(const OrderBook& ob, const OrderBookState& state); // for testing
    OrderBookState getState();
    id_pool getIDPool() {return idPool;}
    audit_list getAuditList() {return auditList;}
    void clear();

    std::list<Order> dummy{{Order::Side::BUY, 3, Order::Type::LIMIT, 50}};

private:

    // for empty pricelevels
    static const order_list emptyOrders;

    // internal processing logic
    OrderResult matchOrder(Order& order);

    // internal data structures
    bid_map bidMap{}; // store active bids
    ask_map askMap{}; // store active asks
    id_map idMap{}; // store map of id : order to get query by id
    trade_list tradeList{}; // store all executed trades
    orders orderList{}; // store a list of orders for bookkeeping
    id_pool idPool{}; // store uuids for ptr persistence and mem efficiency
    audit_list auditList{}; // store record of order modification/cancellation

    double bestBid{-1};
    double bestAsk{-1};
    double marketPrice{-1};
    int totalVolume{};

    const double tickSize{0.01};
};
