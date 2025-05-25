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
    OrderResult matchLimitBuy(Order& order, trades& generatedTrades, OrderResult& default_);
    OrderResult matchLimitSell(Order& order, trades& generatedTrades, OrderResult& default_);
    OrderResult matchMarketBuy(Order& order, trades& generatedTrades);
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

inline std::ostream& operator<<(std::ostream& out, const Order& o)
{
    std::stringstream str;
    str << "Order(id: " << *o.id << ", side: ";

    if (o.side == Order::Side::BUY)
    {
        str << "BUY, ";
    } else if (o.side == Order::Side::SELL)
    {
        str << "SELL, ";
    }

    str << "vol: " << o.volume << ", type: ";

    if (o.type == Order::Type::LIMIT)
    {
        str << "LIMIT, ";
    } else if (o.type == Order::Type::MARKET)
    {
        str << "MARKET, ";
    }

    str << "price: " << std::fixed << o.price << ", timestamp: "
    << o.timestamp << ")";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, const TradeCopy& t)
{
    std::stringstream str;
    str << "Trade(id: " << t.id << ", buyer_id: " << t.buyer_id
    << ", seller_id: " << t.seller_id << ", price: " << t.price
    << ", volume: " << t.volume << ", timestamp: " << t.timestamp
    << ", taker: ";

    switch (t.taker)
    {
        case Order::Side::BUY:
            str << "BUY";
            break;
        case Order::Side::SELL:
            str << "SELL";
            break;
    }

    str << ")";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, const Trade& t)
{
    return out << TradeCopy{t};
}

inline std::ostream& operator<<(std::ostream& out, const trades& t)
{
   std::stringstream str;
   str << "[\n\t";

   for (size_t i = 0; i < t.size(); ++i) {
       str << t[i];
       if (i < t.size() - 1) {
           str << ",\n\t";
       }
   }

   str << "\n]";
   out << str.str();
   return out;
}

inline std::ostream& operator<<(std::ostream& out, const OrderResult& o)
{
    std::stringstream str;
    str << "OrderResult(id: " << o.order_id << ", status: ";

    switch (o.status)
    {
    case OrderResult::PLACED:
        str << "PLACED";
        break;
    case OrderResult::FILLED:
        str << "FILLED";
        break;
    case OrderResult::PARTIALLY_FILLED:
        str << "PARTIALLY_FILLED";
        break;
    case OrderResult::REJECTED:
        str << "REJECTED";
        break;
    case OrderResult::MODIFIED:
        str << "MODIFIED";
        break;
    case OrderResult::CANCELLED:
        str << "CANCELLED";
        break;
    }

    str << ",\n\ttrades: " << o.trades << ",\nremainingOrder: "
    << o.remainingOrder << ", message: " << o.message << ")";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, order_list olist)
{
    std::stringstream str;
    str << "[";

    for (auto itr{olist.begin()}; itr != olist.end(); ++itr) {
        str << *itr;
        str << ", ";
    }

    str << "]";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, PriceLevel p)
{
    std::stringstream str;
    str << "PriceLevel(volume: " << p.volume << ", orders: " << p.orders << ")";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, ask_map a)
{
    std::stringstream str;
    str << "{\n";

    for (auto it{a.begin()}; it != a.end(); ++it)
    {
        str << '\t' << it->first << ": " << it->second << ",\n";
    }

    str << "\n}";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, bid_map a)
{
    std::stringstream str;
    str << "{\n";

    for (auto it{a.begin()}; it != a.end(); ++it)
    {
        str << '\t' << it->first << ": " << it->second << ",\n";
    }

    str << "\n}";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, OrderBook::Level l)
{
    std::stringstream str;
    str << "Level(price: " << l.price << ", volume: " << l.volume
    << ", orderCount: " << l.orderCount << ")";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, OrderBook::Depth d)
{
    std::stringstream str;
    str << "Depth(\n\tbids: [\n\t";

    for (auto bid : d.bids)
    {
        str << '\t' << bid << ",\n\t";
    }

    str << "],\n\tasks: [\n\t";

    for (auto ask : d.asks)
    {
        str << '\t' << ask << ",\n\t";
    }

    str << "],\n\tvolume: " << d.volume << ", bestBid: " << d.bestBid
    << ", bestAsk: " << d.bestAsk << ", marketPrice: " << d.marketPrice
    << "\n)";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, OrderLocation o)
{
    std::stringstream str;
    str << "OrderLocation(order: " << *o.itr << ", price: "
    << o.price << ", side: ";

    if (o.side == Order::Side::BUY) {str << "BUY)";}
    if (o.side == Order::Side::SELL) {str << "SELL)";}

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, id_map i)
{
     std::stringstream str;
    str << "{\n";

    for (auto it{i.begin()}; it != i.end(); ++it)
    {
        str << '\t' << *it->first << ": " << it->second << ",\n";
    }

    str << "\n}";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, trade_list t)
{
    std::stringstream str;
    str << "[";

    for (size_t i = 0; i < t.size(); ++i) {
        str << t[i];
        if (i < t.size() - 1) {
            str << ", ";
        }
    }

    str << "]";
    out << str.str();
    return out;
}

inline std::ostream& operator<<(std::ostream& out, orders o)
{
    std::stringstream str;
    str << "[";

    for (size_t i = 0; i < o.size(); ++i) {
        str << o[i];
        if (i < o.size() - 1) {
            str << ", ";
        }
    }

    str << "]";
    out << str.str();
    return out;
}

inline std::ostream& operator<<(std::ostream& out, OrderBookState s)
{
    std::stringstream str;
    str << "State(\n";
    str << "\tbidMap: " << s.bidMap << ",\n";
    str << "\taskMap: " << s.askMap << ",\n";
    str << "\tidMap: " << s.idMap << ",\n";
    str << "\ttradeList: " << s.tradeList << ",\n";
    str << "\torderList: " << s.orderList << ",\n";
    str << "\tbestBid: " << s.bestBid
    << ", bestAsk: " << s.bestAsk
    << ", marketPrice: " << s.marketPrice
    << ", totalVolume: " << s.totalVolume << "\n)";

    return out << str.str();
}
