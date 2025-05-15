#include "orderbook/Order.h"

// order id and timestamp is going to be initialized when it is passed to the order book
Order::Order(Side side, int volume, Type type, float price)
: side{side}, volume{volume}
, type{type}, price{price}
{
    // Validate based on order type
    if (type == Type::MARKET && price != -1) {
        throw InvalidOrderException{"Market orders cannot specify a price."};
    }

    if (type == Type::LIMIT && price <= 0) {
        throw InvalidOrderException{"Limit orders must specify a (positive) price."};
    }
}

Order Order::makeLimitBuy(int volume, float price)
{
    return {Order::Side::BUY, volume, Order::Type::LIMIT, price};
}

Order Order::makeLimitSell(int volume, float price)
{
    return {Order::Side::SELL, volume, Order::Type::LIMIT, price};
}

Order Order::makeMarketBuy(int volume)
{
    return {Order::Side::BUY, volume, Order::Type::MARKET};
}

Order Order::makeMarketSell(int volume)
{
    return {Order::Side::SELL, volume, Order::Type::MARKET};
}

// for testing
bool Order::equals_to(const Order& other) const
{
    return id == other.id
    && side == other.side
    && volume == other.volume
    && type == other.type
    && price == other.price;
}

bool Order::operator==(const Order& other) const
{
    return id == other.id;
}

const uuids::uuid* Order::get_id() const
{
    return nullptr;
}

void Order::notify(const Trade& trade)
{
    if (callbackFn) {callbackFn(trade);}
}
