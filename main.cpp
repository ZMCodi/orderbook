// for catch testing
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include <unordered_set>
#include <iostream>

#include "Order.h"
#include "Trade.h"
#include "OrderResult.h"
#include "OrderBook.h"

// helper for timestamps
auto time_point() {
    return std::chrono::system_clock::now();
}

// helper for comparing orderlists
bool compareOrderLists(const std::list<Order>& first, const std::list<Order>& sec)
{
    if (first.size() != sec.size()) {return false;}

    auto f{first.begin()};
    auto s{sec.begin()};
    for (std::size_t i{}; i < first.size(); ++i)
    {
        if (!(*f).is_equal(*s)) {return false;}
        ++f;
        ++s;
    }

    return true;
}

TEST_CASE("Order")
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

TEST_CASE("OrderBook")
{
    OrderBook ob{};
    Order buy1{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};
    Order buy2{Order::Side::BUY, 5,  Order::Type::LIMIT, 45};
    Order buy3{Order::Side::BUY, 5,  Order::Type::MARKET};
    Order buy4{Order::Side::BUY, 10, Order::Type::LIMIT, 50};
    Order buy5{Order::Side::BUY, 15, Order::Type::LIMIT, 50};

    Order sell1{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};
    Order sell2{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sell3{Order::Side::SELL, 3,  Order::Type::LIMIT, 55};
    Order sell4{Order::Side::SELL, 30, Order::Type::LIMIT, 60};
    Order sell5{Order::Side::SELL, 27, Order::Type::LIMIT, 60};

    std::vector orders{
        buy1,  buy2,  buy3,  buy4,  buy5,
        sell1, sell2, sell3, sell4, sell5
    };

    SECTION("Takes limit orders and puts them at their price level")
    {
        ob.place_order(buy1);
        ob.place_order(buy2);
        ob.place_order(sell3);
        ob.place_order(sell2);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), std::list{buy1}));
        REQUIRE(compareOrderLists(ob.bidsAt(45.00), std::list{buy2}));
        REQUIRE(compareOrderLists(ob.asksAt(55.00), std::list{sell3}));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), std::list{sell1}));
    }

    SECTION("Handles multiple orders at the same price level")
    {
        // buys at same price
        ob.place_order(buy1);
        ob.place_order(buy4);
        ob.place_order(buy5);

        // sells at same price
        ob.place_order(sell2);
        ob.place_order(sell4);
        ob.place_order(sell5);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), std::list<Order>{
            buy1, buy4, buy5
        }));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), std::list<Order>{
            sell2, sell4, sell5
        }));
    }

    SECTION("Tracks market price")
    {
        REQUIRE_THROWS(ob.getMarketPrice());

        for (auto order : orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getMarketPrice() == Catch::Approx(50.00));
    }

    SECTION("Tracks best bid and best ask")
    {
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
        auto id1{buy1.get_id()};
        ob.place_order(buy1);
        REQUIRE(ob.getOrderByID(id1).is_equal(buy1));
    }

    SECTION("Tracks volume at price level")
    {
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
        REQUIRE(ob.getTotalVolume() == 0);

        for (auto order: orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getTotalVolume() == 100);
    }
}

TEST_CASE("Order Filling")
{
    OrderBook ob{};
    Order buy1{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
    Order buy2{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buy3{Order::Side::BUY, 5, Order::Type::MARKET};

    Order sell1{Order::Side::SELL, 3, Order::Type::LIMIT, 50};
    Order sell2{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sell3{Order::Side::SELL, 3, Order::Type::LIMIT, 55};

    SECTION("Returns the appropriate order status when an order is placed and matched")
    {
        REQUIRE(ob.place_order(buy1).equals_to(OrderResult{buy1.get_id(), OrderResult::PLACED, std::vector<Trade>(), &buy1, ""}));
        REQUIRE(ob.place_order(sell1).equals_to(OrderResult{sell1.get_id(), OrderResult::FILLED, std::vector<Trade>{
            Trade{"", buy1.get_id(), sell1.get_id(), 50, 3, time_point(), Order::Side::SELL}
        }, nullptr, ""}));

    }

    SECTION("Reject market order when there is not enough liquidity")
    {
        REQUIRE(ob.place_order(buy3).equals_to(OrderResult{buy3.get_id(), OrderResult::REJECTED, std::vector<Trade>(), &buy3, "Not enough liquidity"}));
    }
}

// TODO:
// ORDER BOOK:
// empty order book behaviour
// modify order
// cancel full/partially filled order
// depth
// EDGE CASES:
// identical price levels
// ORDER:
// market order with partial rejection
// partial filling market/limit order
// walking the book market/limit order
// time priority within price level
