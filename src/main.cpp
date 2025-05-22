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

int main()
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};

    auto res{ob.placeOrder(buy50)};
    OrderResult exp{
        *buy50.get_id(), 
        OrderResult::PLACED, 
        trades(), 
        &ob.getOrderByID(buy50.get_id()), 
        "Order placed"
    };

    std::cout << res << "\n\n";
    std::cout << exp;
}
