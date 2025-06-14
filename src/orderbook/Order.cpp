#include "orderbook/Order.h"

// order id and timestamp is going to be initialized when it is passed to the order book
Order::Order(Side side, int volume, Type type, double price, double stopPrice)
: id{nullptr}, side{side}, volume{volume}
, type{type}, price{price}, stopPrice{stopPrice}
{
    // Validate based on order type
    if ((type == Type::LIMIT || type == Type::STOP_LIMIT) && price <= 0)
    {
        throw std::invalid_argument{"Limit orders must specify a (positive) price."};
    }

    if ((type == Type::STOP_LIMIT || type == Type::STOP) && stopPrice <= 0)
    {
        throw std::invalid_argument{"Stop orders must specify a (positive) stop price."};
    }

    if (volume <= 0)
    {
        throw std::invalid_argument{"Volume has to be positive"};
    }
}

Order::Order(const Order& order, double tickSize)
: id{order.id}, side{order.side}, volume{order.volume}
, type{order.type}, price{utils::trunc(order.price, tickSize)}
, stopPrice{utils::trunc(order.stopPrice, tickSize)}
, timestamp{order.timestamp} {}

Order Order::makeLimitBuy(int volume, double price)
{
    return {Order::Side::BUY, volume, Order::Type::LIMIT, price};
}

Order Order::makeLimitSell(int volume, double price)
{
    return {Order::Side::SELL, volume, Order::Type::LIMIT, price};
}

Order Order::makeMarketBuy(int volume)
{
    return {Order::Side::BUY, volume, Order::Type::MARKET, -1};
}

Order Order::makeMarketSell(int volume)
{
    return {Order::Side::SELL, volume, Order::Type::MARKET, -1};
}

Order Order::makeStopBuy(int volume, double stopPrice)
{
    return {Order::Side::BUY, volume, Order::Type::STOP, -1, stopPrice};
}

Order Order::makeStopSell(int volume, double stopPrice)
{
    return {Order::Side::SELL, volume, Order::Type::STOP, -1, stopPrice};
}

Order Order::makeStopLimitBuy(int volume, double price, double stopPrice)
{
    return {Order::Side::BUY, volume, Order::Type::STOP_LIMIT, price, stopPrice};
}

Order Order::makeStopLimitSell(int volume, double price, double stopPrice)
{
    return {Order::Side::SELL, volume, Order::Type::STOP_LIMIT, price, stopPrice};
}

// for testing
// dont compare timestamp since copies cant possibly have the same timestamp
// but id can be compared since we can get id from getOrderByID() or OrderResult
bool Order::equals_to(const Order& other) const
{
    return id == other.id
    && side == other.side
    && volume == other.volume
    && type == other.type
    && std::abs(price - other.price) < 0.0001
    && std::abs(stopPrice - other.stopPrice) < 0.0001;
}

bool Order::operator==(const Order& other) const
{
    return id == other.id;
}

const uuids::uuid* Order::get_id() const
{
    return id;
}
