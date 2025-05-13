#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "test_helpers.h"

TEST_CASE("Order", "[order]")
{
    OrderBook ob{};

    SECTION("Market order with a price throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 3, Order::Type::MARKET, 50});
        REQUIRE_THROWS(Order{Order::Side::SELL, 3, Order::Type::MARKET, 50});
    }

    SECTION("Limit order without a price throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 3, Order::Type::LIMIT});
        REQUIRE_THROWS(Order{Order::Side::SELL, 3, Order::Type::LIMIT});
    }

    SECTION("Zero or negative volume orders throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 0, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::SELL, 0, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::BUY, 0, Order::Type::MARKET});
        REQUIRE_THROWS(Order{Order::Side::SELL, 0, Order::Type::MARKET});

        REQUIRE_THROWS(Order{Order::Side::BUY, -1, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::SELL, -1, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::BUY, -1, Order::Type::MARKET});
        REQUIRE_THROWS(Order{Order::Side::SELL, -1, Order::Type::MARKET});
    }

    SECTION("Zero or negative price limit orders throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 20, Order::Type::LIMIT, 0});
        REQUIRE_THROWS(Order{Order::Side::SELL, 20, Order::Type::LIMIT, 0});
        REQUIRE_THROWS(Order{Order::Side::BUY, 20, Order::Type::LIMIT, -1});
        REQUIRE_THROWS(Order{Order::Side::SELL, 20, Order::Type::LIMIT, -1});
    }

    SECTION("Order ID's generated are unique")
    {
        // Create persistent Order objects first
        Order order1{Order::Side::BUY, 1, Order::Type::LIMIT, 1};
        Order order2{Order::Side::BUY, 1, Order::Type::LIMIT, 1};
        Order order3{Order::Side::SELL, 1, Order::Type::MARKET};
        Order order4{Order::Side::BUY, 1, Order::Type::MARKET};

        // Now get the IDs from the persistent objects
        auto id1 = order1.get_id();
        auto id2 = order2.get_id();
        auto id3 = order3.get_id();
        auto id4 = order4.get_id();

        std::unordered_set<std::string_view> ids{
            id1, id2, id3, id4
        };

        REQUIRE(ids.size() == 4);
    }

    SECTION("Order handles large volume")
    {
        int max_vol{std::numeric_limits<int>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, max_vol, Order::Type::LIMIT, 1});
    }

    SECTION("Order handles large and small prices")
    {
        float max_price{std::numeric_limits<float>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, max_price});
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, 0.01f});
    }
}
