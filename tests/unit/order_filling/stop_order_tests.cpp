#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Stop orders", "[order filling][stop]")
{
    OrderBook ob{};
    Order SB55{Order::makeStopBuy(5, 55)};
    Order SLB50_55{Order::makeStopLimitBuy(5, 50, 55)};

    Order SS50{Order::makeStopSell(5, 50)};
    Order SLS45_50{Order::makeStopLimitSell(5, 45, 50)};

    SECTION("Placing stop orders")
    {
        auto actual1{ob.placeOrder(SB55)};
        auto actual2{ob.placeOrder(SLB50_55)};
        auto actual3{ob.placeOrder(SS50)};
        auto actual4{ob.placeOrder(SLS45_50)};

        OrderResult expected1{
            *SB55.id,
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(SB55.id),
            "Order placed"
        };

        OrderResult expected2{
            *SLB50_55.id,
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(SLB50_55.id),
            "Order placed"
        };

        OrderResult expected3{
            *SS50.id,
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(SS50.id),
            "Order placed"
        };

        OrderResult expected4{
            *SLS45_50.id,
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(SLS45_50.id),
            "Order placed"
        };

        REQUIRE(actual1.equals_to(expected1));
        REQUIRE(actual2.equals_to(expected2));
        REQUIRE(actual3.equals_to(expected3));
        REQUIRE(actual4.equals_to(expected4));
    }

    SECTION("Stop orders go to stop map and not bid/ask map")
    {
        ob.placeOrder(SB55);
        ob.placeOrder(SLB50_55);
        ob.placeOrder(SS50);
        ob.placeOrder(SLS45_50);

        auto [actSBM, actSSM] = ob.getStopMaps();

        // volume doesnt matter for stop maps
        stop_buy_map expSBM{
            {5500, {0, order_list{SB55, SLB50_55}}}
        };

        stop_sell_map expSSM{
            {5000, {0, order_list{SS50, SLS45_50}}}
        };

        REQUIRE(compareMaps(actSBM, expSBM));
        REQUIRE(compareMaps(actSSM, expSSM));

        id_map expIDM{
            {SB55.id, OrderLocation{55, expSBM.at(5500).orders.begin(), OrderLocation::STOP_BUY}},
            {SB55.id, OrderLocation{55, ++expSBM.at(5500).orders.begin(), OrderLocation::STOP_BUY}},
            {SB55.id, OrderLocation{55, expSSM.at(5000).orders.begin(), OrderLocation::STOP_SELL}},
            {SB55.id, OrderLocation{55, ++expSSM.at(5000).orders.begin(), OrderLocation::STOP_SELL}},
        };

        OrderBookState expState{
            bid_map(), ask_map(), expIDM, trade_list(),
            orders{SB55, SLB50_55, SS50, SLS45_50},
            -1, -1, -1, 0 // stop orders dont count towards ob volume
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Market price movements trigger stop orders correctly")
    {

    }

    SECTION("Misaligned stop orders trigger immediately")
    {

    }

}
