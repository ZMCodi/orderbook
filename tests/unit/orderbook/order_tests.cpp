#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "test_helpers.h"

static const time_ epoch{};


void assertOrder(Order& o, Order::Side side, int vol, Order::Type type,
                double price, double stopPrice)
{
    REQUIRE(!o.id);
    REQUIRE(o.side == side);
    REQUIRE(o.volume == vol);
    REQUIRE(o.type == type);
    REQUIRE(o.price == Catch::Approx(price));
    REQUIRE(o.stopPrice == Catch::Approx(stopPrice));
    REQUIRE(o.timestamp == epoch);
    REQUIRE(!o.getCallback());
}

TEST_CASE("Order", "[order]")
{
    SECTION("Basic order initialization checks")
    {
        Order order1{Order::makeLimitBuy(1, 20)};
        Order order2{Order::makeLimitSell(1, 20)};
        Order order3{Order::makeMarketBuy(1)};
        Order order4{Order::makeMarketSell(1)};
        Order order5{Order::makeStopBuy(1, 20)};
        Order order6{Order::makeStopSell(1, 20)};
        Order order7{Order::makeStopLimitBuy(1, 20, 15)};
        Order order8{Order::makeStopLimitSell(1, 20, 25)};

        using Order::Side::BUY, Order::Side::SELL;

        assertOrder(order1, BUY, 1, Order::Type::LIMIT, 20.0, -1.0);
        assertOrder(order2, SELL, 1, Order::Type::LIMIT, 20.0, -1.0);
        assertOrder(order3, BUY, 1, Order::Type::MARKET, -1.0, -1.0);
        assertOrder(order4, SELL, 1, Order::Type::MARKET, -1.0, -1.0);
        assertOrder(order5, BUY, 1, Order::Type::STOP, -1.0, 20.0);
        assertOrder(order6, SELL, 1, Order::Type::STOP, -1.0, 20.0);
        assertOrder(order7, BUY, 1, Order::Type::STOP_LIMIT, 20.0, 15.0);
        assertOrder(order8, SELL, 1, Order::Type::STOP_LIMIT, 20.0, 25.0);
    }

    SECTION("Zero or negative volume orders throws an error")
    {
        REQUIRE_THROWS(Order::makeLimitBuy(0, 20));
        REQUIRE_THROWS(Order::makeLimitSell(0, 20));
        REQUIRE_THROWS(Order::makeMarketBuy(0));
        REQUIRE_THROWS(Order::makeMarketSell(0));
        REQUIRE_THROWS(Order::makeStopBuy(0, 20));
        REQUIRE_THROWS(Order::makeStopSell(0, 20));
        REQUIRE_THROWS(Order::makeStopLimitBuy(0, 20, 15));
        REQUIRE_THROWS(Order::makeStopLimitSell(0, 20, 25));

        REQUIRE_THROWS(Order::makeLimitBuy(-1, 20));
        REQUIRE_THROWS(Order::makeLimitSell(-1, 20));
        REQUIRE_THROWS(Order::makeMarketBuy(-1));
        REQUIRE_THROWS(Order::makeMarketSell(-1));
        REQUIRE_THROWS(Order::makeStopBuy(-1, 20));
        REQUIRE_THROWS(Order::makeStopSell(-1, 20));
        REQUIRE_THROWS(Order::makeStopLimitBuy(-1, 20, 15));
        REQUIRE_THROWS(Order::makeStopLimitSell(-1, 20, 25));
    }

    SECTION("Zero or negative price limit/stop limit orders throws an error")
    {
        REQUIRE_THROWS(Order::makeLimitBuy(20, 0));
        REQUIRE_THROWS(Order::makeLimitSell(20, 0));
        REQUIRE_THROWS(Order::makeLimitBuy(20, -1));
        REQUIRE_THROWS(Order::makeLimitSell(20, -1));
        REQUIRE_THROWS(Order::makeStopLimitBuy(20, 0, 10));
        REQUIRE_THROWS(Order::makeStopLimitSell(20, 0, 10));
        REQUIRE_THROWS(Order::makeStopLimitBuy(20, -1, 10));
        REQUIRE_THROWS(Order::makeStopLimitSell(20, -1, 10));
    }

    SECTION("Order handles large and small volumes")
    {
        int max_vol{std::numeric_limits<int>::max()};
        REQUIRE_NOTHROW(Order::makeLimitBuy(max_vol, 1));
        REQUIRE_NOTHROW(Order::makeLimitBuy(1, 1));
        REQUIRE_NOTHROW(Order::makeMarketBuy(max_vol));
        REQUIRE_NOTHROW(Order::makeMarketBuy(1));
        REQUIRE_NOTHROW(Order::makeStopLimitBuy(max_vol, 1, 1));
        REQUIRE_NOTHROW(Order::makeStopLimitBuy(1, 1, 1));
        REQUIRE_NOTHROW(Order::makeStopBuy(max_vol, 1));
        REQUIRE_NOTHROW(Order::makeStopBuy(1, 1));
    }

    SECTION("Order handles large and small prices")
    {
        double max_price{std::numeric_limits<double>::max()};
        REQUIRE_NOTHROW(Order::makeLimitBuy(1, max_price));
        REQUIRE_NOTHROW(Order::makeLimitBuy(1, 0.0001));
        REQUIRE_NOTHROW(Order::makeStopLimitBuy(1, max_price, max_price));
        REQUIRE_NOTHROW(Order::makeStopLimitBuy(1, 0.0001, 0.0001));
        REQUIRE_NOTHROW(Order::makeStopBuy(1, max_price));
        REQUIRE_NOTHROW(Order::makeStopBuy(1, 0.0001));
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
