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
    Order buy1{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
    Order buy2{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buy3{Order::Side::BUY, 5, Order::Type::MARKET};

    Order sell1{Order::Side::SELL, 3, Order::Type::LIMIT, 50};
    Order sell2{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sell3{Order::Side::SELL, 3, Order::Type::LIMIT, 55};

    // change this to return a linked list of orders at the price level
    SECTION("Takes limit orders and puts them at their price level")
    {
        ob.place_order(buy1);
        ob.place_order(buy2);
        ob.place_order(sell3);
        ob.place_order(sell2);

        REQUIRE(ob.bidAt(50.00).is_equal(buy1));
        REQUIRE(ob.bidAt(45.00).is_equal(buy2));
        REQUIRE(ob.askAt(55.00).is_equal(sell3));
        REQUIRE(ob.askAt(60.00).is_equal(sell1));
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
        REQUIRE(ob.place_order(buy1) == OrderResult{buy1.get_id(), OrderResult::PLACED, std::vector<Trade>(), &buy1, ""});
        REQUIRE(ob.place_order(sell1) == OrderResult{sell1.get_id(), OrderResult::FILLED, std::vector<Trade>{
            Trade{"", buy1.get_id(), sell1.get_id(), 50, 3, time_point(), Order::Side::SELL}
        }, nullptr, ""});

    }

    SECTION("Reject market order when there is not enough liquidity")
    {
        REQUIRE(ob.place_order(buy3) == OrderResult{buy3.get_id(), OrderResult::REJECTED, std::vector<Trade>(), &buy3, "Not enough liquidity"});
    }
}

// TODO:
// ORDER BOOK:
// best bid/ask and tracking after trades
// volume at price level
// empty order book behaviour
// market price updates
// get order by id
// modify order
// cancel full/partially filled order
// total volume
// depth
// EDGE CASES:
// identical price levels
// ORDER:
// market order with partial rejection
// partial filling market/limit order
// walking the book market/limit order
// time priority within price level
