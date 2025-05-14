#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("OrderBook", "[orderbook][basic]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5,  Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 5,  Order::Type::MARKET};
    Order buy50_2{Order::Side::BUY, 10, Order::Type::LIMIT, 50};
    Order buy50_3{Order::Side::BUY, 15, Order::Type::LIMIT, 50};

    Order sell50{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};
    Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sell55{Order::Side::SELL, 3,  Order::Type::LIMIT, 55};
    Order sell60_2{Order::Side::SELL, 30, Order::Type::LIMIT, 60};
    Order sell60_3{Order::Side::SELL, 27, Order::Type::LIMIT, 60};

    std::vector orders{
        buy50,  buy45,  buyMarket,  buy50_2,  buy50_3,
        sell50, sell60, sell55, sell60_2, sell60_3
    };

    SECTION("Takes limit orders and puts them at their price level")
    {
        ob.place_order(buy50);
        ob.place_order(buy45);
        ob.place_order(sell55);
        ob.place_order(sell60);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), std::list{buy50}));
        REQUIRE(compareOrderLists(ob.bidsAt(45.00), std::list{buy45}));
        REQUIRE(compareOrderLists(ob.asksAt(55.00), std::list{sell55}));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), std::list{sell60}));
    }

    SECTION("Handles multiple orders at the same price level")
    {
        // buys at same price
        ob.place_order(buy50);
        ob.place_order(buy50_2);
        ob.place_order(buy50_3);

        // sells at same price
        ob.place_order(sell60);
        ob.place_order(sell60_2);
        ob.place_order(sell60_3);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), std::list<Order>{
            buy50, buy50_2, buy50_3
        }));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), std::list<Order>{
            sell60, sell60_2, sell60_3
        }));
    }

    SECTION("Tracks market price")
    {
        // market price not initialized
        REQUIRE_THROWS(ob.getMarketPrice());

        for (auto order : orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getMarketPrice() == Catch::Approx(50.00));
    }

    SECTION("Tracks best bid and best ask")
    {
        // best bid/ask not initialized
        REQUIRE_THROWS(ob.getBestBid());
        REQUIRE_THROWS(ob.getBestAsk());

        for (auto order: orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getBestBid() == Catch::Approx(50.00));
        REQUIRE(ob.getBestAsk() == Catch::Approx(55.00));
    }

    SECTION("Gets order by ID")
    {
        auto id1{buy50.id};
        ob.place_order(buy50);
        REQUIRE(ob.getOrderByID(id1).equals_to(buy50));
    }

    SECTION("Tracks volume at price level")
    {
        // empty should be 0
        REQUIRE(ob.volumeAt(50.00) == 0);

        for (auto order: orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.volumeAt(60.00) == 67);
        REQUIRE(ob.volumeAt(55.00) == 3);
        REQUIRE(ob.volumeAt(50.00) == 25);
        REQUIRE(ob.volumeAt(45.00) == 5);
    }

    SECTION("Tracks total volume")
    {
        // empty should be 0
        REQUIRE(ob.getTotalVolume() == 0);

        for (auto order: orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getTotalVolume() == 100);
    }

    SECTION("Spread")
    {
        REQUIRE_THROWS(ob.getSpread());

        ob.place_order(buy50);
        REQUIRE_THROWS(ob.getSpread());

        ob.place_order(sell55);

        REQUIRE(ob.getSpread() == Catch::Approx(5.00f));
    }
}
