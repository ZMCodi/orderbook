#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "test_helpers.h"

TEST_CASE("Order", "[order]")
{
    SECTION("Basic order initialization checks")
    {
        Order order1{Order::makeLimitBuy(1, 20)};
        Order order2{Order::makeLimitSell(1, 20)};
        Order order3{Order::makeMarketBuy(1)};
        Order order4{Order::makeMarketSell(1)};
        // TODO: add stop and stop limit tests

        time_ epoch{};

        REQUIRE(!order1.id);
        REQUIRE(order1.side == Order::Side::BUY);
        REQUIRE(order1.volume == 1);
        REQUIRE(order1.type == Order::Type::LIMIT);
        REQUIRE(order1.price == Catch::Approx(20.0));
        REQUIRE(order1.stopPrice == Catch::Approx(-1.0));
        REQUIRE(order1.timestamp == epoch);
        REQUIRE(!order1.getCallback());

        REQUIRE(!order2.id);
        REQUIRE(order2.side == Order::Side::SELL);
        REQUIRE(order2.volume == 1);
        REQUIRE(order2.type == Order::Type::LIMIT);
        REQUIRE(order2.price == Catch::Approx(20.0));
        REQUIRE(order1.stopPrice == Catch::Approx(-1.0));
        REQUIRE(order2.timestamp == epoch);
        REQUIRE(!order2.getCallback());

        REQUIRE(!order3.id);
        REQUIRE(order3.side == Order::Side::BUY);
        REQUIRE(order3.volume == 1);
        REQUIRE(order3.type == Order::Type::MARKET);
        REQUIRE(order3.price == Catch::Approx(-1.0));
        REQUIRE(order1.stopPrice == Catch::Approx(-1.0));
        REQUIRE(order3.timestamp == epoch);
        REQUIRE(!order3.getCallback());

        REQUIRE(!order4.id);
        REQUIRE(order4.side == Order::Side::SELL);
        REQUIRE(order4.volume == 1);
        REQUIRE(order4.type == Order::Type::MARKET);
        REQUIRE(order4.price == Catch::Approx(-1.0));
        REQUIRE(order1.stopPrice == Catch::Approx(-1.0));
        REQUIRE(order4.timestamp == epoch);
        REQUIRE(order4.getCallback() == nullptr);
    }

    SECTION("Zero or negative volume orders throws an error")
    {
        REQUIRE_THROWS(Order::makeLimitBuy(0, 20));
        REQUIRE_THROWS(Order::makeLimitSell(0, 20));
        REQUIRE_THROWS(Order::makeMarketBuy(0));
        REQUIRE_THROWS(Order::makeMarketSell(0));

        REQUIRE_THROWS(Order::makeLimitBuy(-1, 20));
        REQUIRE_THROWS(Order::makeLimitSell(-1, 20));
        REQUIRE_THROWS(Order::makeMarketBuy(-1));
        REQUIRE_THROWS(Order::makeMarketSell(-1));
    }

    SECTION("Zero or negative price limit orders throws an error")
    {
        REQUIRE_THROWS(Order::makeLimitBuy(20, 0));
        REQUIRE_THROWS(Order::makeLimitSell(20, 0));
        REQUIRE_THROWS(Order::makeLimitBuy(20, -1));
        REQUIRE_THROWS(Order::makeLimitSell(20, -1));

    }

    SECTION("Order handles large and small volumes")
    {
        int max_vol{std::numeric_limits<int>::max()};
        REQUIRE_NOTHROW(Order::makeLimitBuy(max_vol, 1));
        REQUIRE_NOTHROW(Order::makeLimitBuy(1, 1));
        REQUIRE_NOTHROW(Order::makeMarketBuy(max_vol));
        REQUIRE_NOTHROW(Order::makeMarketBuy(1));
    }

    SECTION("Order handles large and small prices")
    {
        double max_price{std::numeric_limits<double>::max()};
        REQUIRE_NOTHROW(Order::makeLimitBuy(1, max_price));
        REQUIRE_NOTHROW(Order::makeLimitBuy(1, 0.0001));
    }

    SECTION("Check price precision")
    {
        Order order1{Order::makeLimitBuy(1, 0.12)};
        Order order2{Order::makeLimitBuy(1, 0.1234)};

        REQUIRE(!order1.equals_to(order2));
        REQUIRE(order1.price == Catch::Approx(0.12).epsilon(0.01)); // 2 dp average price precision
        REQUIRE(order2.price == Catch::Approx(0.1234).epsilon(0.0001)); // 4 dp for penny stocks
    }
}
