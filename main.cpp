// for catch testing
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include "Order.h"
#include "Trade.h"
#include "OrderResult.h"
#include "OrderBook.h"

// helper for timestamps
auto time_point() {
    return std::chrono::system_clock::now();
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

    SECTION("Returns the appropriate order status when an order is placed and matched")
    {
        // unmatched order
        // REQUIRE(ob.place_order(buy1) == OrderResult{buy1.get_id(), OrderResult::PLACED, std::vector<Trade>(), &buy1, ""});
        REQUIRE(ob.place_order(sell1) == OrderResult{sell1.get_id(), OrderResult::FILLED, std::vector<Trade>{
            Trade{"", buy1.get_id(), sell1.get_id(), 50, 3, time_point(), Order::Side::SELL}
        }, nullptr, ""});

    }

    SECTION("Reject market order when there is not enough liquidity")
    {
        REQUIRE(ob.place_order(buy3) == OrderResult{buy3.get_id(), OrderResult::REJECTED, std::vector<Trade>(), &buy3, "Not enough liquidity"});
    }

    // change this to return a list of orders at the price level
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

    // TODO:
    // ORDER:
    // market order
    // market order with partial rejection
    // partial filling market/limit order
    // walking the book market/limit order
    // time priority within price level
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
    // zero volume orders
    // large volume orders
    // extremely high/low prices
    // identical price levels
}
