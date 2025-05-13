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
    Order::Side agressor;

    // for testing
    bool is_equal(const Trade& other) const
    {
        return buyer_id == other.buyer_id
        && seller_id == other.seller_id
        && price == other.price
        && volume == other.volume
        && agressor == other.agressor;
    }

    bool operator==(const Trade& other) const
    {
        return id == other.id;
    }
};
