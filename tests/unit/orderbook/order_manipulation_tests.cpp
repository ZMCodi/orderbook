#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Order cancellation/modification", "[orderbook][manipulation]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 5,  Order::Type::LIMIT, 50};
    Order buy50_2{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5,  Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 10,  Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 5,  Order::Type::LIMIT, 50};
    Order sell50_2{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};
    Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sellMarket{Order::Side::SELL, 5,  Order::Type::MARKET};

    const uuids::uuid* id;

    SECTION("Cancel full limit buy")
    {
        ob.place_order(buy50);
        id = buy50.get_id();
        auto actual{ob.cancel_order(id)};

        OrderResult expected{
            id,
            OrderResult::CANCELLED,
            trade_ptrs(),
            nullptr,
            "Order cancelled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list(),
            -1, -1, -1, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Cancel full limit sell")
    {
        ob.place_order(sell50);
        id = sell50.get_id();
        auto actual{ob.cancel_order(id)};

        OrderResult expected{
            id,
            OrderResult::CANCELLED,
            trade_ptrs(),
            nullptr,
            "Order cancelled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list(),
            -1, -1, -1, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Cancel partial limit buy")
    {
        ob.place_order(buy50);
        id = buy50.get_id();
        ob.place_order(sell50_2); // this will partially fill buy50
        auto actual{ob.cancel_order(id)};

        OrderResult expected{
            id,
            OrderResult::CANCELLED,
            trade_ptrs(),
            nullptr,
            "Order cancelled with 2 unfilled shares"
        };

        REQUIRE(actual.equals_to(expected));

        Trade expTrade{nullptr, id, sell50_2.get_id(), 50, 3, time_point(), Order::Side::SELL};
        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Cancel partial limit sell")
    {
        ob.place_order(sell50);
        id = sell50.get_id();
        ob.place_order(buy50_2);
        auto actual{ob.cancel_order(id)};

        OrderResult expected{
            id,
            OrderResult::CANCELLED,
            trade_ptrs(),
            nullptr,
            "Order cancelled with 2 unfilled shares"
        };

        REQUIRE(actual.equals_to(expected));

        Trade expTrade{nullptr, buy50_2.get_id(), id, 50, 3, time_point(), Order::Side::BUY};
        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Increase volume full limit buy")
    {

    }

    SECTION("Increase volume full limit sell")
    {

    }

    SECTION("Increase volume partial limit buy")
    {

    }

    SECTION("Increase volume partial limit sell")
    {

    }

    SECTION("Decrease volume full limit buy")
    {

    }

    SECTION("Decrease volume full limit sell")
    {

    }

    SECTION("Decrease volume partial limit buy")
    {

    }

    SECTION("Decrease volume partial limit sell")
    {

    }

    SECTION("Modify price full limit buy")
    {

    }

    SECTION("Modify price full limit sell")
    {

    }

    SECTION("Modify price partial limit buy")
    {

    }

    SECTION("Modify price partial limit sell")
    {

    }
}
