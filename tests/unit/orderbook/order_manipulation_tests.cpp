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

    SECTION("Decreasing volume to negative throws error")
    {
        ob.place_order(buy50);
        REQUIRE_THROWS(ob.modify_volume(buy50.get_id(), -1));
    }

    // decreasing volume should maintain time priority
    SECTION("Decrease volume full limit buy")
    {
        ob.place_order(buy50);
        id = buy50.get_id();
        ob.place_order(buy50_2);
        auto actual{ob.modify_volume(id, 2)};

        OrderResult expected{
            id,
            OrderResult::MODIFIED,
            trade_ptrs(),
            &ob.getOrderByID(id),
            "Volume decreased from 5 to 2"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        buy50.volume = 2; // ob doesnt change original object

        bid_map expBM{
            {50.0, PriceLevel{5, order_list{buy50, buy50_2}}}
        };

        id_map expIDM{
            {buy50.get_id(), OrderLocation{50.0, expBM.at(50.0).orders.begin(), Order::Side::BUY}},
            {buy50_2.get_id(), OrderLocation{50.0, ++expBM.at(50.0).orders.begin(), Order::Side::BUY}},
        };

        OrderBookState expState{
            expBM, ask_map(), expIDM, trade_list(),
            50, -1, -1, 5
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Decrease volume full limit sell")
    {
        ob.place_order(sell50);
        id = sell50.get_id();
        ob.place_order(sell50_2);
        auto actual{ob.modify_volume(id, 2)};

        OrderResult expected{
            id,
            OrderResult::MODIFIED,
            trade_ptrs(),
            &ob.getOrderByID(id),
            "Volume decreased from 5 to 2"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        sell50.volume = 2; // ob doesnt change original object

        ask_map expAM{
            {50.0, PriceLevel{5, order_list{sell50, sell50_2}}}
        };

        id_map expIDM{
            {sell50.get_id(), OrderLocation{50.0, expAM.at(50.0).orders.begin(), Order::Side::SELL}},
            {sell50_2.get_id(), OrderLocation{50.0, ++expAM.at(50.0).orders.begin(), Order::Side::SELL}},
        };

        OrderBookState expState{
            bid_map(), expAM, expIDM, trade_list(),
            -1, 50, -1, 5
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Decrease volume partial limit buy")
    {
        ob.place_order(buy50);
        id = buy50.get_id();
        // this fills 3 of buy50's volume
        Trade expTrade{*ob.place_order(sell50_2).trades[0]};
        ob.place_order(buy50_2);
        auto actual{ob.modify_volume(id, 1)};

        OrderResult expected{
            id,
            OrderResult::MODIFIED,
            trade_ptrs(),
            &ob.getOrderByID(id),
            "Volume decreased from 2 to 1"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 1);
        buy50.volume = 1; // ob doesnt change original object

        bid_map expBM{
            {50.0, PriceLevel{4, order_list{buy50, buy50_2}}}
        };

        id_map expIDM{
            {buy50.get_id(), OrderLocation{50.0, expBM.at(50.0).orders.begin(), Order::Side::BUY}},
            {buy50_2.get_id(), OrderLocation{50.0, ++expBM.at(50.0).orders.begin(), Order::Side::BUY}},
        };

        OrderBookState expState{
            expBM, ask_map(), expIDM, trade_list{expTrade},
            50, -1, 50, 4
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Decrease volume partial limit sell")
    {
        ob.place_order(sell50);
        id = sell50.get_id();
        // this fills 3 of sell50's volume
        Trade expTrade{*ob.place_order(buy50_2).trades[0]};
        ob.place_order(sell50_2);
        auto actual{ob.modify_volume(id, 1)};

        OrderResult expected{
            id,
            OrderResult::MODIFIED,
            trade_ptrs(),
            &ob.getOrderByID(id),
            "Volume decreased from 2 to 1"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 1);
        sell50.volume = 1; // ob doesnt change original object

        bid_map expAM{
            {50.0, PriceLevel{4, order_list{sell50, sell50_2}}}
        };

        id_map expIDM{
            {sell50.get_id(), OrderLocation{50.0, expAM.at(50.0).orders.begin(), Order::Side::SELL}},
            {sell50_2.get_id(), OrderLocation{50.0, ++expAM.at(50.0).orders.begin(), Order::Side::SELL}},
        };

        OrderBookState expState{
            bid_map(), expAM, expIDM, trade_list{expTrade},
            -1, 50, 50, 4
        };

        REQUIRE(checkOBState(ob, expState));
    }

    // increase volume pushes order to the back
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
