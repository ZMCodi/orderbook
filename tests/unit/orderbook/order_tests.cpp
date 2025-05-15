#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "test_helpers.h"

TEST_CASE("Order", "[order]")
{
    SECTION("Basic order initialization checks")
    {
        Order order1{Order::Side::BUY, 1, Order::Type::LIMIT, 20};
        Order order2{Order::Side::SELL, 1, Order::Type::LIMIT, 20};
        Order order3{Order::Side::BUY, 1, Order::Type::MARKET};
        Order order4{Order::Side::SELL, 1, Order::Type::MARKET};

        std::chrono::time_point<std::chrono::system_clock> epoch{};

        REQUIRE(!order1.id);
        REQUIRE(order1.side == Order::Side::BUY);
        REQUIRE(order1.volume == 1);
        REQUIRE(order1.type == Order::Type::LIMIT);
        REQUIRE(order1.price == Catch::Approx(20.0));
        REQUIRE(order1.timestamp == epoch);
        REQUIRE(!order1.callbackFn);

        REQUIRE(!order2.id);
        REQUIRE(order2.side == Order::Side::SELL);
        REQUIRE(order2.volume == 1);
        REQUIRE(order2.type == Order::Type::LIMIT);
        REQUIRE(order2.price == Catch::Approx(20.0));
        REQUIRE(order2.timestamp == epoch);
        REQUIRE(!order2.callbackFn);

        REQUIRE(!order3.id);
        REQUIRE(order3.side == Order::Side::BUY);
        REQUIRE(order3.volume == 1);
        REQUIRE(order3.type == Order::Type::MARKET);
        REQUIRE(order3.price == Catch::Approx(-1.0));
        REQUIRE(order3.timestamp == epoch);
        REQUIRE(!order3.callbackFn);

        REQUIRE(!order4.id);
        REQUIRE(order4.side == Order::Side::SELL);
        REQUIRE(order4.volume == 1);
        REQUIRE(order4.type == Order::Type::MARKET);
        REQUIRE(order4.price == Catch::Approx(-1.0));
        REQUIRE(order4.timestamp == epoch);
        REQUIRE(order4.callbackFn == nullptr);
    }

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

    SECTION("Order handles large and small volumes")
    {
        int max_vol{std::numeric_limits<int>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, max_vol, Order::Type::LIMIT, 1});
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, 1});
        REQUIRE_NOTHROW(Order{Order::Side::BUY, max_vol, Order::Type::MARKET});
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::MARKET});
    }

    SECTION("Order handles large and small prices")
    {
        float max_price{std::numeric_limits<float>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, max_price});
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, 0.01f});
    }

    SECTION("Create orders using factory function")
    {
        Order limitBuy{Order{Order::Side::BUY, 1, Order::Type::LIMIT, 50}};
        Order limitSell{Order{Order::Side::SELL, 1, Order::Type::LIMIT, 50}};
        Order marketBuy{Order{Order::Side::BUY, 1, Order::Type::MARKET}};
        Order marketSell{Order{Order::Side::SELL, 1, Order::Type::MARKET}};

        REQUIRE(Order::makeLimitBuy(1, 50).equals_to(limitBuy));
        REQUIRE(Order::makeLimitSell(1, 50).equals_to(limitSell));
        REQUIRE(Order::makeMarketBuy(1).equals_to(marketBuy));
        REQUIRE(Order::makeMarketSell(1).equals_to(marketSell));
    }

    SECTION("Check price precision")
    {
        Order order1{Order::Side::BUY, 1, Order::Type::LIMIT, 0.123456789};
        Order order2{Order::Side::BUY, 1, Order::Type::LIMIT, 0.1234567890123456789};

        REQUIRE(!order1.equals_to(order2));
        REQUIRE(order1.price == Catch::Approx(0.123456789).epsilon(0.000000001));
        REQUIRE(order2.price == Catch::Approx(0.1234567890123456789).epsilon(0.000000001));
    }
}
