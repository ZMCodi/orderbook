#pragma once

#include <array>
#include <string>
#include <string_view>
#include <chrono>
#include <exception>

#include "libraries/uuid.h"
#include "libraries/Random.h"

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

    Side side;
    int volume;
    Type type;
    float price;
    std::chrono::time_point<std::chrono::system_clock> timestamp;

private:
    // id should be private since its represented internally
    // as int instead of uuid
    uint64_t id;
};
