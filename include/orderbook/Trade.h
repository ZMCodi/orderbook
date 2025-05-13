#pragma once

#include "Order.h"

struct Trade
{
    std::string id{uuids::to_string(uuid_generator())};
    std::string_view buyer_id;
    std::string_view seller_id;
    float price;
    int volume;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    Order::Side taker;

    bool is_equal(const Trade& other) const; // for testing
    bool operator==(const Trade& other) const;
};
