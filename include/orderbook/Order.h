#pragma once

#include <array>
#include <string>
#include <string_view>
#include <exception>
#include <functional>

#include "libraries/uuid.h"
#include "libraries/Random.h"
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

    Order(Side side, int volume, Type type, float price = -1);

    // factory functions
    static Order makeLimitBuy(int volume, float price);
    static Order makeLimitSell(int volume, float price);
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
    const float price;
    time_ timestamp;

    friend class OrderBook;
    friend Order truncPrice(const Order& order, float tickSize); // helper for testing

private:
    Order(const Order& order, float tickSize);
    void notify(Trade trade);

    callback callbackFn;
};
