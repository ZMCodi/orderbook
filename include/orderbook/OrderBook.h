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

    // OrderResult modifyVolume(const uuids::uuid* id, int volume);
    OrderResult modifyPrice(const uuids::uuid* id, double price);

    // reference overload
    // OrderResult modifyVolume(const uuids::uuid& id, int volume);
    OrderResult modifyPrice(const uuids::uuid& id, double price);

    // templates
    OrderResult cancelOrder(auto&& id_);
    OrderResult modifyVolume(auto&& id_, int volume);

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
    const id_pool& getIDPool() {return idPool;}
    audit_list getAuditList() {return auditList;}
    void clear();

    std::list<Order> dummy{{Order::Side::BUY, 3, Order::Type::LIMIT, 50}};

private:

    // for empty pricelevels
    static const order_list emptyOrders;

    // internal processing logic
    auto dispatchBySide(Order::Side side, auto&& func);
    const uuids::uuid* getPointer(auto&& id, bool throws);

    OrderResult matchOrder(Order& order);
    template<Order::Type OrderType>
    OrderResult matchOrderTemplate(Order& order, auto& orderMap);

    void genTrade(const Order& buyer, const Order& seller, double price,
                  int volume, Order::Side side, trades& generatedTrades);

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

template<Order::Type OrderType>
OrderResult OrderBook::matchOrderTemplate(Order& order, auto& orderMap)
{
    OrderResult default_{*order.id, OrderResult::PLACED, trades{}, nullptr, "Order placed"};
    OrderResult rejected{*order.id, OrderResult::REJECTED, trades{}, nullptr, "Not enough liquidity"};
    trades generatedTrades{}; // trades generated by matches

    // convenience variable to easily track what type of order we're working with
    // buy order will walk askMap, sell order will walk bidMap
    // use ternary here because side has to be initialized using constexpr
    constexpr Order::Side side = std::is_same_v<decltype(orderMap), ask_map&> ? Order::Side::BUY : Order::Side::SELL;

    if constexpr (OrderType == Order::Type::MARKET)
    {
        if constexpr(side == Order::Side::BUY) // market buy
        {
            if (bestAsk == -1) {return rejected;} // no liquidity
        } else // market sell
        {
            if (bestBid == -1) {return rejected;}
        }
    } else
    {
        if constexpr(side == Order::Side::BUY) // limit buy
        {
            // no asks to match
            if (bestAsk == -1 || order.price < bestAsk) {return default_;}
        } else // limit sell
        {
            // no bids to match
            if (bestBid == -1 || order.price > bestBid) {return default_;}
        }
    }

    int oriVol{order.volume}; // save here for result

    // start at best bid/ask
    for (auto mapIt{orderMap.begin()}; mapIt != orderMap.end();/*increment logic is complex*/)
    {
        // end map walking logic only for limit since market will keep matching
        if constexpr (OrderType == Order::Type::LIMIT)
        {
            if constexpr(side == Order::Side::BUY) // limit buy
            {
                // order has filled all orders below its price
                if (mapIt->first * tickSize > order.price) {break;}
            } else // limit sell
            {
                // order has filled all orders above its price
                if (mapIt->first * tickSize < order.price) {break;}
            }
        }

        // iterate through orders at the level
        order_list& orders{mapIt->second.orders};
        for (auto orderIt{orders.begin()}; orderIt != orders.end();/*increment logic is complex*/)
        {
            auto& o{*orderIt}; // current order being matched against

            // determine who is buyer/seller
            Order& buyer = side == Order::Side::BUY ? order : o;
            Order& seller = side == Order::Side::BUY ? o : order;

            if (order.volume >= o.volume) // order gets partial filled by o
            {
                order.volume -= o.volume;
                genTrade(buyer, seller, o.price, o.volume, order.side, generatedTrades);

                // remove o since it is matched
                idMap.erase(o.id); // from idMap
                mapIt->second.volume -= o.volume; // decrese volume at price level
                totalVolume -= o.volume; // and total volume
                orderIt = orders.erase(orderIt); // remove o and increment orderIt

            } else // order gets fully filled
            {
                o.volume -= order.volume;
                genTrade(buyer, seller, o.price, order.volume, order.side, generatedTrades);

                // update price level and total volume
                mapIt->second.volume -= order.volume;
                totalVolume -= order.volume;
                order.volume = 0;
            }

            if (order.volume == 0) {break;} // order is filled
        }

        // clear level if no more orders left and increment
        if (mapIt->second.orders.empty()) {mapIt = orderMap.erase(mapIt);}
        else {++mapIt;}
    }

    // update best bid/ask
    // determine which to update based on order side
    double& bid_or_ask = side == Order::Side::BUY ? bestAsk : bestBid;
    if (orderMap.empty()) {bid_or_ask = -1;}
    else {bid_or_ask = orderMap.begin()->first * tickSize;}

    // return OrderResult
    if (order.volume == 0) // fully filled
    {
        return {*order.id, OrderResult::FILLED, generatedTrades, nullptr, "Order filled"};
    } else // partially filled
    {
        // different messages for limit/market
        std::stringstream msg;
        msg << "Partially filled " << oriVol - order.volume << " shares, ";
        if constexpr (OrderType == Order::Type::LIMIT)
        {
            msg << order.volume << " shares remaining";
        } else
        {
            msg << "remaining order cancelled";
        }

        return {*order.id, OrderResult::PARTIALLY_FILLED, generatedTrades, nullptr, msg.str()};
    }
}

auto OrderBook::dispatchBySide(Order::Side side, auto&& func)
{
    return side == Order::Side::BUY ? func(bidMap) : func(askMap);
}

const uuids::uuid* OrderBook::getPointer(auto&& id, bool throws)
{
    // id passed in is a reference
    if constexpr (std::is_same_v<std::decay_t<decltype(id)>, uuids::uuid>)
    {
        id_pool::iterator it{idPool.find(id)};

        if (it == idPool.end()) // not found
        {
            if (throws) {throw std::invalid_argument{"ID does not exist"};}
            return nullptr;
        }

        return &(*it);
    } else // already a pointer
    {
        return id;
    }
}

OrderResult OrderBook::cancelOrder(auto&& id_)
{
    const uuids::uuid* id{getPointer(id_, true)};

    auto [price, itr, side] = idMap.at(id); // unpack the OrderLocation
    auto tickPrice{utils::convertTick(price, tickSize)};
    auto vol{itr->volume};

    // update volume and idMap
    totalVolume -= vol;
    idMap.erase(id);

    // remove order from bid/ask map and update best bid/ask
    dispatchBySide(side, [&](auto& orderMap){
        orderMap.at(tickPrice).volume -= vol;
        orderMap.at(tickPrice).orders.erase(itr);

        // clear pricelevel if now empty
        if (orderMap.at(tickPrice).orders.empty()) {orderMap.erase(tickPrice);}

        // update best bid/ask
        double& bid_or_ask = std::is_same_v<decltype(orderMap), bid_map&> ? bestBid : bestAsk;
        if (orderMap.empty()) {bid_or_ask = -1;}
        else {bid_or_ask = orderMap.begin()->first * tickSize;}
    });

    // update auditList
    auditList.emplace_back(id, utils::now(), -1);

    std::stringstream msg;
    msg << "Order cancelled with " << vol << " unfilled shares";
    return {*id, OrderResult::CANCELLED, trades(), nullptr, msg.str()};
}

OrderResult OrderBook::modifyVolume(auto&& id_, int volume)
{
    if (volume <= 0)
    {
        throw std::invalid_argument{"Volume has to be positive"};
    }

    const uuids::uuid* id{getPointer(id_, true)};

    auto [price, itr, side] = idMap.at(id); // unpack the OrderLocation
    auto vol{itr->volume};
    auto delta{vol - volume};
    auto tickPrice{utils::convertTick(price, tickSize)};


    if (delta == 0) // unchanged
    {
        return {*id, OrderResult::REJECTED, trades{}, &(*itr), "Volume unchanged"};
    }

    // return OrderResult
    if (delta > 0)
    {
        totalVolume -= delta;

        // decrease volume maintain time priority
        dispatchBySide(side, [&](auto& orderMap){
            itr->volume -= delta;
            orderMap.at(tickPrice).volume -= delta;
        });

        auditList.emplace_back(id, utils::now(), delta);

        std::stringstream msg;
        msg << "Volume decreased from " << vol << " to " << volume;
        return {*id, OrderResult::MODIFIED, trades{}, &(*itr), msg.str()};

    } else // cancel and replace
    {
        Order replace{*itr}; // copy with modified volume
        replace.volume -= delta;
        auto cb{replace.callbackFn};

        cancelOrder(id); // cancel
        auto placeRes{placeOrder(replace, cb)}; // replace

        std::stringstream msg;
        msg << "Volume increased from " << vol << " to " << volume
        << ". New ID generated.";
        return {*placeRes.remainingOrder->id, OrderResult::MODIFIED, trades{},
                placeRes.remainingOrder, msg.str()};
    }
}
