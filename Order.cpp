#include "Order.h"

Order::Order(Side side, int volume, Type type, float price)
    : id{uuids::to_string(uuid_generator())}
    , side{side}, volume{volume}
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
bool Order::is_equal(const Order& other) const
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
