#pragma once

#include <array>
#include <string>
#include <string_view>
#include <chrono>
#include <exception>

#include <uuid.h>
#include "Random.h"

inline std::mt19937 engine{Random::generate()};
inline uuids::uuid_random_generator uuid_generator(engine);

class Order
{
public:
    enum class Side
    {
        BUY,
        SELL
    };
    static constexpr std::array<std::string, 2> side_str{"BUY", "SELL"};

    enum class Type
    {
        LIMIT,
        MARKET
    };
    static constexpr std::array<std::string, 2> type_str{"LIMIT", "MARKET"};

    class InvalidOrderException : public std::invalid_argument {
    public:
        InvalidOrderException(const std::string& message) 
            : std::invalid_argument(message) {}
    };

    Order(Side side, int volume, Type type, float price = -1)
        : id{uuids::to_string(uuid_generator())}
        , side{side}
        , volume{volume}
        , type{type}
        , price{price}
        , timestamp{std::chrono::system_clock::now()} 
    {
        // Validate based on order type
        if (type == Type::MARKET && price != -1) {
            throw InvalidOrderException{"Market orders cannot specify a price."};
        }

        if (type == Type::LIMIT && price == -1) {
            throw InvalidOrderException{"Limit orders must specify a price."};
        }
    }

    friend std::ostream& operator<<(std::ostream& out, const Order& order)
    {
        return out << "Order(SIDE:" << Order::side_str[static_cast<std::size_t>(order.side)] << ", VOL:" << order.volume
            << ", TYPE:" << Order::type_str[static_cast<std::size_t>(order.type)] << ", PRICE:" << order.price
            << " PLACED_AT:" << order.timestamp << ")";
    }

    // for testing
    bool is_equal(const Order& other) const
    {
        return id == other.id
        && side == other.side
        && volume == other.volume
        && type == other.type
        && price == other.price;
    }

    bool operator==(const Order& other) const
    {
        return id == other.id;
    }

    std::string_view get_id() {return id;}

private:
    std::string id;
    Side side;
    int volume;
    Type type;
    float price;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};
