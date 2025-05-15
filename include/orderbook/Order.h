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

using callback = std::function<void(const Trade&)>;

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
    bool equals_to(const Order& other) const; // for testing
    bool operator==(const Order& other) const;
    const uuids::uuid* get_id();
    void notify(const Trade& trade);

    const uuids::uuid* id;
    Side side;
    int volume;
    Type type;
    float price;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    callback callbackFn;
};
