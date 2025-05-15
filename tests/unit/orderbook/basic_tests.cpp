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
    Order sellMarket{Order::Side::SELL, 5,  Order::Type::MARKET};

    std::vector orders{
        buy50,  buy45,  buyMarket,  buy50_2,  buy50_3,
        sell50, sell60, sell55, sell60_2, sell60_3
    };

    SECTION("Gets order by ID")
    {
        ob.placeOrder(buy50);
        auto id1{buy50.get_id()};
        REQUIRE(ob.getOrderByID(id1).equals_to(buy50));

        // nonexistent ID
        auto fakeID{uuid_generator()};
        REQUIRE_THROWS(ob.getOrderByID(fakeID));
        REQUIRE_THROWS(ob.getOrderByID(nullptr));
    }

    SECTION("Placing orders")
    {
        REQUIRE_NOTHROW(ob.placeOrder(buy50));
        REQUIRE_NOTHROW(ob.placeOrder(sell60));
        REQUIRE_NOTHROW(ob.placeOrder(buyMarket));
        REQUIRE_NOTHROW(ob.placeOrder(sellMarket));
        REQUIRE_NOTHROW(ob.placeOrder({Order::Side::BUY, 5, Order::Type::LIMIT, 40}));
    }

    SECTION("Place buy order")
    {
        auto actual{ob.placeOrder(buy50)};
        OrderResult expected{
            buy50.get_id(), 
            OrderResult::PLACED, 
            trade_ptrs(), 
            &ob.getOrderByID(buy50.get_id()), 
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Place sell order")
    {
        auto actual{ob.placeOrder(sell50)};
        OrderResult expected{
            sell50.get_id(), 
            OrderResult::PLACED, 
            trade_ptrs(), 
            &ob.getOrderByID(sell50.get_id()), 
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Place buy order with rvalue")
    {
        auto actual{ob.placeOrder(Order::makeLimitBuy(5, 50))};
        OrderResult expected{
            actual.order_id, // hack here since we cant get the actual id
            OrderResult::PLACED, 
            trade_ptrs(), 
            actual.remainingOrder, 
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Place sell order with rvalue")
    {
        auto actual{ob.placeOrder(Order::makeLimitSell(5, 50))};
        OrderResult expected{
            actual.order_id, // hack here since we cant get the actual id
            OrderResult::PLACED, 
            trade_ptrs(), 
            actual.remainingOrder, 
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Takes limit orders and puts them at their price level")
    {
        ob.placeOrder(buy50);
        ob.placeOrder(buy45);
        ob.placeOrder(sell55);
        ob.placeOrder(sell60);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), order_list{buy50}));
        REQUIRE(compareOrderLists(ob.bidsAt(45.00), order_list{buy45}));
        REQUIRE(compareOrderLists(ob.asksAt(55.00), order_list{sell55}));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), order_list{sell60}));
    }

    SECTION("Re-placing the same order")
    {
        ob.placeOrder(buy45);
        auto actual{ob.placeOrder(buy45)};
        auto old_id{actual.order_id};

        OrderResult expected{
            buy45.get_id(),
            OrderResult::REJECTED,
            trade_ptrs(),
            &ob.getOrderByID(buy45.get_id()),
            "Order already exists"
        };
        REQUIRE(actual.equals_to(expected));

        // to re-place an identical order, set id to nullptr
        buy45.id = nullptr;
        auto actual2{ob.placeOrder(buy45)};
        REQUIRE(*old_id != *actual.order_id); // new ID is generated

        OrderResult expected2{
            buy45.get_id(),
            OrderResult::PLACED,
            trade_ptrs(),
            &ob.getOrderByID(buy45.get_id()),
            ""
        };
        REQUIRE(actual2.equals_to(expected2));

        // order with existing ID is also rejected
        auto fakeID{uuid_generator()};
        sell50.id = &fakeID;
        auto actual3{ob.placeOrder(sell50)};

        OrderResult expected3{
            &fakeID, // no new ID is generated
            OrderResult::REJECTED,
            trade_ptrs(),
            &sell50,
            "Non-null ID"
        };
        REQUIRE(actual3.equals_to(expected3));
    }

    SECTION("Tracks market price")
    {
        // market price not initialized
        REQUIRE_THROWS(ob.getMarketPrice());

        for (auto order : orders)
        {
            ob.placeOrder(order);
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
            ob.placeOrder(order);
        }

        REQUIRE(ob.getBestBid() == Catch::Approx(50.00));
        REQUIRE(ob.getBestAsk() == Catch::Approx(55.00));
    }

    SECTION("Tracks volume at price level")
    {
        // empty should be 0
        REQUIRE(ob.volumeAt(50.00) == 0);

        for (auto order: orders)
        {
            ob.placeOrder(order);
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
            ob.placeOrder(order);
        }

        REQUIRE(ob.getTotalVolume() == 100);
    }

    SECTION("Spread")
    {
        REQUIRE_THROWS(ob.getSpread());

        ob.placeOrder(buy50);
        REQUIRE_THROWS(ob.getSpread());

        ob.placeOrder(sell55);

        REQUIRE(ob.getSpread() == Catch::Approx(5.00f));
    }
}
