#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("OrderBook", "[orderbook][basic]")
{
    OrderBook ob{};
    Order buy50{Order::makeLimitBuy(3, 50)};
    Order buy45{Order::makeLimitBuy(5, 45)};
    Order buy50_2{Order::makeLimitBuy(10, 50)};
    Order buy50_3{Order::makeLimitBuy(15, 50)};
    Order buyMarket{Order::makeMarketBuy(5)};

    Order sell50{Order::makeLimitSell(3,  50)};
    Order sell60{Order::makeLimitSell(10, 60)};
    Order sell55{Order::makeLimitSell(3,  55)};
    Order sell60_2{Order::makeLimitSell(30, 60)};
    Order sell60_3{Order::makeLimitSell(27, 60)};
    Order sellMarket{Order::makeMarketSell(5)};

    std::vector<std::reference_wrapper<Order>> orders_{
        buy50,  buy45,  buyMarket,  buy50_2,  buy50_3,
        sell50, sell60, sell55, sell60_2, sell60_3
    };

    SECTION("Empty orderbook state")
    {
        REQUIRE(checkOBState(ob, emptyState));
        REQUIRE(ob.getStopMaps().first.empty());
        REQUIRE(ob.getStopMaps().second.empty());
        REQUIRE(ob.getIDPool().empty());

        // check clear method
        for (auto order : orders_)
        {
            ob.placeOrder(order);
        }

        ob.placeOrder(Order::makeStopBuy(1, 20));
        ob.placeOrder(Order::makeStopSell(1, 20));
        ob.placeOrder(Order::makeStopLimitBuy(1, 20, 20));
        ob.placeOrder(Order::makeStopLimitSell(1, 20, 20));

        ob.clear();
        REQUIRE(checkOBState(ob, emptyState));
        REQUIRE(ob.getStopMaps().first.empty());
        REQUIRE(ob.getStopMaps().second.empty());
        REQUIRE(ob.getIDPool().empty());
    }

    SECTION("Gets order by ID")
    {
        ob.placeOrder(buy50);
        auto id1{buy50.id};
        REQUIRE(ob.getOrderByID(id1).equals_to(buy50));

        // nonexistent ID
        auto fakeID{utils::uuid_generator()};
        REQUIRE_THROWS(ob.getOrderByID(fakeID));
        REQUIRE_THROWS(ob.getOrderByID(nullptr));
    }

    SECTION("Placing orders")
    {
        REQUIRE_NOTHROW(ob.placeOrder(buy50));
        REQUIRE_NOTHROW(ob.placeOrder(sell60));
        REQUIRE_NOTHROW(ob.placeOrder(buyMarket));
        REQUIRE_NOTHROW(ob.placeOrder(sellMarket));
        REQUIRE_NOTHROW(ob.placeOrder(Order::makeLimitBuy(5, 40)));
        REQUIRE_NOTHROW(ob.placeOrder(Order::makeStopBuy(1, 20)));
        REQUIRE_NOTHROW(ob.placeOrder(Order::makeStopSell(1, 20)));
        REQUIRE_NOTHROW(ob.placeOrder(Order::makeStopLimitBuy(1, 20, 20)));
        REQUIRE_NOTHROW(ob.placeOrder(Order::makeStopLimitSell(1, 20, 20)));
    }

    SECTION("Reject invalid orders")
    {
        // change manually since order constructor will not allow this
        buy50.volume = sell60.volume = buyMarket.volume = sellMarket.volume = 0;
        REQUIRE_THROWS(ob.placeOrder(buy50));
        REQUIRE_THROWS(ob.placeOrder(sell60));
        REQUIRE_THROWS(ob.placeOrder(buyMarket));
        REQUIRE_THROWS(ob.placeOrder(sellMarket));
    }

    SECTION("Place buy order")
    {
        auto actual{ob.placeOrder(buy50)};
        OrderResult expected{
            *buy50.id, 
            OrderResult::PLACED, 
            trades(), 
            &ob.getOrderByID(buy50.id), 
            "Order placed"
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Place sell order")
    {
        auto actual{ob.placeOrder(sell50)};
        OrderResult expected{
            *sell50.id, 
            OrderResult::PLACED, 
            trades(), 
            &ob.getOrderByID(sell50.id), 
            "Order placed"
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Place buy order with rvalue")
    {
        auto actual{ob.placeOrder(Order::makeLimitBuy(5, 50))};
        OrderResult expected{
            actual.order_id, // hack here since we cant get the actual id
            OrderResult::PLACED, 
            trades(), 
            actual.remainingOrder, 
            "Order placed"
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Place sell order with rvalue")
    {
        auto actual{ob.placeOrder(Order::makeLimitSell(5, 50))};
        OrderResult expected{
            actual.order_id, // hack here since we cant get the actual id
            OrderResult::PLACED, 
            trades(), 
            actual.remainingOrder, 
            "Order placed"
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Takes limit orders and puts them at their price level")
    {
        REQUIRE(compareOrderLists(ob.bidsAt(50.00), order_list()));
        REQUIRE(compareOrderLists(ob.bidsAt(45.00), order_list()));
        REQUIRE(compareOrderLists(ob.asksAt(55.00), order_list()));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), order_list()));

        // new ordersAt tests
        REQUIRE(compareOrderLists(ob.ordersAt(50.00), order_list()));
        REQUIRE(compareOrderLists(ob.ordersAt(45.00), order_list()));
        REQUIRE(compareOrderLists(ob.ordersAt(55.00), order_list()));
        REQUIRE(compareOrderLists(ob.ordersAt(60.00), order_list()));

        ob.placeOrder(buy50);
        ob.placeOrder(buy45);
        ob.placeOrder(sell55);
        ob.placeOrder(sell60);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), order_list{buy50}));
        REQUIRE(compareOrderLists(ob.bidsAt(45.00), order_list{buy45}));
        REQUIRE(compareOrderLists(ob.asksAt(55.00), order_list{sell55}));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), order_list{sell60}));

        REQUIRE(compareOrderLists(ob.ordersAt(50.00), order_list{buy50}));
        REQUIRE(compareOrderLists(ob.ordersAt(45.00), order_list{buy45}));
        REQUIRE(compareOrderLists(ob.ordersAt(55.00), order_list{sell55}));
        REQUIRE(compareOrderLists(ob.ordersAt(60.00), order_list{sell60}));
    }

    SECTION("Price level precision")
    {
        // these should be on different price levels
        auto order5001_1{Order::makeLimitBuy(5, 50.01)};
        auto order5002_1{Order::makeLimitBuy(5, 50.02)};
        ob.placeOrder(order5001_1);
        ob.placeOrder(order5002_1);

        // but these should be on 50.01
        auto order5001_2{Order::makeLimitBuy(5, 50.0101)};
        auto order5001_3{Order::makeLimitBuy(5, 50.0162)}; // this should truncate, not round up
        ob.placeOrder(order5001_2);
        ob.placeOrder(order5001_3);

        // check that they are rounded
        REQUIRE(ob.getOrderByID(order5001_1.id).price == Catch::Approx(50.01).epsilon(0.01));
        REQUIRE(ob.getOrderByID(order5001_2.id).price == Catch::Approx(50.01).epsilon(0.01));
        REQUIRE(ob.getOrderByID(order5001_3.id).price == Catch::Approx(50.01).epsilon(0.01));

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), order_list()));
        REQUIRE(compareOrderLists(ob.bidsAt(50.01), order_list{order5001_1, order5001_2, order5001_3}));
        REQUIRE(compareOrderLists(ob.bidsAt(50.02), order_list{order5002_1}));

        REQUIRE(compareOrderLists(ob.ordersAt(50.00), order_list()));
        REQUIRE(compareOrderLists(ob.ordersAt(50.01), order_list{order5001_1, order5001_2, order5001_3}));
        REQUIRE(compareOrderLists(ob.ordersAt(50.02), order_list{order5002_1}));

        // now for an ob with a different precision
        OrderBook ob2{0.001};

        // these should be the same price level (50.010)
        ob2.placeOrder(order5001_1);
        ob2.placeOrder(order5001_2);

        // but this is a different price level (50.016)
        ob2.placeOrder(order5001_3);

        // check that they are rounded
        REQUIRE(ob2.getOrderByID(order5001_1.id).price == Catch::Approx(50.010).epsilon(0.001));
        REQUIRE(ob2.getOrderByID(order5001_2.id).price == Catch::Approx(50.010).epsilon(0.001));
        REQUIRE(ob2.getOrderByID(order5001_3.id).price == Catch::Approx(50.016).epsilon(0.001));

        REQUIRE(compareOrderLists(ob2.bidsAt(50.01), order_list{order5001_1, order5001_2}, 0.001));
        REQUIRE(compareOrderLists(ob2.bidsAt(50.010), order_list{order5001_1, order5001_2}, 0.001));
        REQUIRE(compareOrderLists(ob2.bidsAt(50.016), order_list{order5001_3}, 0.001));

        REQUIRE(compareOrderLists(ob2.ordersAt(50.01), order_list{order5001_1, order5001_2}, 0.001));
        REQUIRE(compareOrderLists(ob2.ordersAt(50.010), order_list{order5001_1, order5001_2}, 0.001));
        REQUIRE(compareOrderLists(ob2.ordersAt(50.016), order_list{order5001_3}, 0.001));
    }

    SECTION("Small and large prices")
    {
        double max_price{999'999.99}; // highest traded stock is ~700k

        auto order1{Order::makeLimitBuy(5, max_price)};
        auto order2{Order::makeLimitBuy(5, 0.01)};

        ob.placeOrder(order1);
        ob.placeOrder(order2);

        auto truncPrice{utils::trunc(max_price, 0.01)};
        REQUIRE(compareOrderLists(ob.bidsAt(truncPrice), order_list{order1}));
        REQUIRE(compareOrderLists(ob.ordersAt(truncPrice), order_list{order1}));
        REQUIRE(ob.getOrderByID(order1.id).price == Catch::Approx(truncPrice).epsilon(0.01));

        REQUIRE(compareOrderLists(ob.bidsAt(0.01), order_list{order2}));
        REQUIRE(compareOrderLists(ob.ordersAt(0.01), order_list{order2}));
        REQUIRE(ob.getOrderByID(order2.id).price == Catch::Approx(0.01).epsilon(0.01));
    }

    SECTION("Tracks market price")
    {
        // market price not initialized
        REQUIRE_THROWS(ob.getMarketPrice());

        for (auto order : orders_)
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

        for (auto order: orders_)
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

        for (auto order: orders_)
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

        for (auto order: orders_)
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

        REQUIRE(ob.getSpread() == Catch::Approx(5.00));
    }
}
