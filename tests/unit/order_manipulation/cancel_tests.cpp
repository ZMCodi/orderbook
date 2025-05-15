#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Order cancellation", "[order manipulation][cancellation]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 5,  Order::Type::LIMIT, 50};
    Order buy50_2{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};

    Order sell50{Order::Side::SELL, 5,  Order::Type::LIMIT, 50};
    Order sell50_2{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};

    const uuids::uuid* id;

    SECTION("Cancel full limit buy")
    {
        ob.placeOrder(buy50);
        id = buy50.get_id();
        auto actual{ob.cancelOrder(id)};

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
        ob.placeOrder(sell50);
        id = sell50.get_id();
        auto actual{ob.cancelOrder(id)};

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
        ob.placeOrder(buy50);
        id = buy50.get_id();
        ob.placeOrder(sell50_2); // this will partially fill buy50
        auto actual{ob.cancelOrder(id)};

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
        ob.placeOrder(sell50);
        id = sell50.get_id();
        ob.placeOrder(buy50_2);
        auto actual{ob.cancelOrder(id)};

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
}
