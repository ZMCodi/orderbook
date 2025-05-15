#pragma once

#include <array>
#include <string>
#include <string_view>
#include <chrono>
#include <exception>
#include <functional>

#include "libraries/uuid.h"
#include "libraries/Random.h"

struct Trade;

using callback = std::function<void(Trade)>;
using time_ = std::chrono::time_point<std::chrono::system_clock>;

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

    class InvalidOrderException : public std::invalid_argument {
    public:
        InvalidOrderException(const std::string& message) 
            : std::invalid_argument(message) {}
    };

    Order(Side side, int volume, Type type, float price = -1);

    // factory functions
    static Order makeLimitBuy(int volume, float price);
    static Order makeLimitSell(int volume, float price);
    static Order makeMarketBuy(int volume);
    static Order makeMarketSell(int volume);

    bool equals_to(const Order& other) const; // for testing
    bool operator==(const Order& other) const;
    const uuids::uuid* get_id() const;
    void notify(Trade trade);

    const uuids::uuid* id;
    const Side side;
    int volume;
    const Type type;
    const float price;
    time_ timestamp;
    callback callbackFn;
};
