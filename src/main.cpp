#include <iostream>
#include <sstream>

#include "orderbook/OrderBook.h"

inline OrderBookState OrderBook::getState()
{
    return {
        bidMap, askMap, idMap, tradeList, orderList,
        bestBid, bestAsk, marketPrice, totalVolume
    };
}

std::ostream& operator<<(std::ostream& out, const Order& o)
{
    std::stringstream str;
    str << "Order(id: " << o.id << ", side: ";

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

    str << "price: " << o.price << ", timestamp: "
    << o.timestamp << ")";

    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, const Trade& t)
{
    std::stringstream str;
    str << "Trade(id: " << *t.id << ", buyer_id: " << *t.buyer_id
    << ", seller_id: " << *t.seller_id << ", price: " << t.price
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

std::ostream& operator<<(std::ostream& out, const trades& t)
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

std::ostream& operator<<(std::ostream& out, const OrderResult& o)
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

    str << ", trades: " << o.trades << ", remainingOrder: "
    << o.remainingOrder << ", message: " << o.message << ")";

    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, order_list olist)
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

std::ostream& operator<<(std::ostream& out, PriceLevel p)
{
    std::stringstream str;
    str << "PriceLevel(volume: " << p.volume << ", orders: " << p.orders << ")";
    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, ask_map a)
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

std::ostream& operator<<(std::ostream& out, bid_map a)
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

std::ostream& operator<<(std::ostream& out, OrderBook::Level l)
{
    std::stringstream str;
    str << "Level(price: " << l.price << ", volume: " << l.volume
    << ", orderCount: " << l.orderCount << ")";

    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, OrderBook::Depth d)
{
    std::stringstream str;
    str << "Depth(bids: [";

    for (auto bid : d.bids)
    {
        str << bid << ", ";
    }

    str << "], asks: [";

    for (auto ask : d.asks)
    {
        str << ask << ", ";
    }

    str << "], volume: " << d.volume << ", bestBid: " << d.bestBid
    << ", bestAsk: " << d.bestAsk << ", marketPrice: " << d.marketPrice
    << ")";

    return out << str.str();
}

int main()
{
    OrderBook ob{};

    // Setup some test orders at various price levels
    Order buyLow{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buyLow2{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buyLow3{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buyMid{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
    Order buyMid2{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
    Order buyHigh{Order::Side::BUY, 7, Order::Type::LIMIT, 55};

    Order sellLow{Order::Side::SELL, 4, Order::Type::LIMIT, 60};
    Order sellLow2{Order::Side::SELL, 4, Order::Type::LIMIT, 60};
    Order sellLow3{Order::Side::SELL, 4, Order::Type::LIMIT, 60};
    Order sellMid{Order::Side::SELL, 6, Order::Type::LIMIT, 65};
    Order sellMid2{Order::Side::SELL, 6, Order::Type::LIMIT, 65};
    Order sellHigh{Order::Side::SELL, 8, Order::Type::LIMIT, 70};

    // Place orders to populate the book
    ob.placeOrder(buyLow);
    ob.placeOrder(buyLow2);
    ob.placeOrder(buyLow3);
    ob.placeOrder(buyMid);
    ob.placeOrder(buyMid2);
    ob.placeOrder(buyHigh);
    ob.placeOrder(sellLow);
    ob.placeOrder(sellLow2);
    ob.placeOrder(sellLow3);
    ob.placeOrder(sellMid);
    ob.placeOrder(sellMid2);
    ob.placeOrder(sellHigh);

    std::cout << ob.getDepthInRange(47.5, 62.5);
}
