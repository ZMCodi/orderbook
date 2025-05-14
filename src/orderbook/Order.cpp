#include "orderbook/Order.h"

// order id is going to be initialized when it is passed to the order book
Order::Order(Side side, int volume, Type type, float price)
    : side{side}, volume{volume}
    , type{type}, price{price}
    , timestamp{std::chrono::system_clock::now()} 
{
    // Validate based on order type
    if (type == Type::MARKET && price != -1) {
        throw InvalidOrderException{"Market orders cannot specify a price."};
    }

    if (type == Type::LIMIT && price <= 0) {
        throw InvalidOrderException{"Limit orders must specify a (positive) price."};
    }
}

// for testing
bool Order::equals_to(const Order& other) const
{
    return id == other.id
    && side == other.side
    && volume == other.volume
    && type == other.type
    && price == other.price
    && timestamp == other.timestamp;
}

bool Order::operator==(const Order& other) const
{
    return id == other.id;
}

const uuids::uuid* Order::get_id()
{
    return nullptr;
}
