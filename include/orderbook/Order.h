#pragma once

#include <array>
#include <string>
#include <string_view>
#include <chrono>
#include <exception>

#include "libraries/uuid.h"
#include "libraries/Random.h"

inline static std::mt19937 engine{Random::generate()};
inline static uuids::uuid_random_generator uuid_generator(engine);

struct Order
{
public:
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

    std::string id;
    Side side;
    int volume;
    Type type;
    float price;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};
