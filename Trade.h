#pragma once

#include "Order.h"

struct Trade
{
    std::string id{uuids::to_string(uuid_generator())};
    std::string_view buy_id;
    std::string_view sell_id;
    float price;
    int volume;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    Order::Side agressor;

    // for testing
    bool is_equal(const Trade& other) const
    {
        return buy_id == other.buy_id
        && sell_id == other.sell_id
        && price == other.price
        && volume == other.volume
        && agressor == other.agressor;
    }

    bool operator==(const Trade& other) const
    {
        return id == other.id;
    }
};
