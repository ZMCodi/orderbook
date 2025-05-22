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

int main()
{

}
