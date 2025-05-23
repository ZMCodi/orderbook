#include <iostream>
#include <sstream>

#include "orderbook/OrderBook.h"

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

int main()
{
    OrderBook ob{0.001f};
    // these should be on different price levels
    auto order5001_1{Order::makeLimitBuy(5, 50.01f)};
    auto order5002_1{Order::makeLimitBuy(5, 50.02f)};
    ob.placeOrder(order5001_1);
    ob.placeOrder(order5002_1);

    // but these should be on 50.01
    auto order5001_2{Order::makeLimitBuy(5, 50.0101f)};
    auto order5001_3{Order::makeLimitBuy(5, 50.0162f)}; // this should truncate, not round up
    ob.placeOrder(order5001_2);
    ob.placeOrder(order5001_3);

    std::cout << ob.bidsAt(50.01f) << '\n';
    std::cout.precision(10);
    std::cout << std::floor(50.01 / 0.001) * 0.001 << '\n';
    std::cout << std::floor(50.01f / 0.001f) * 0.001f << '\n';
}
