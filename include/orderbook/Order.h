#pragma once

#include <exception>
#include <functional>

#include "orderbook/Utils.h"

struct Trade;
class OrderBook;

using callback = std::function<void(Trade)>;

struct Order
{
    enum class Side
    {
        BUY,
        SELL
    };

    enum class Type
    {
        LIMIT,
        MARKET
    };

    Order(Side side, int volume, Type type, double price = -1);

    // factory functions
    static Order makeLimitBuy(int volume, double price);
    static Order makeLimitSell(int volume, double price);
    static Order makeMarketBuy(int volume);
    static Order makeMarketSell(int volume);

    bool equals_to(const Order& other) const; // for testing
    callback getCallback() {return callbackFn;} // for testing
    bool operator==(const Order& other) const;
    const uuids::uuid* get_id() const;

    const uuids::uuid* id;
    const Side side;
    int volume;
    const Type type;
    const double price;
    time_ timestamp;

    friend class OrderBook;
    friend Order truncPrice(const Order& order, double tickSize); // helper for testing

private:
    Order(const Order& order, double tickSize);
    void notify(Trade trade);

    callback callbackFn;
};
