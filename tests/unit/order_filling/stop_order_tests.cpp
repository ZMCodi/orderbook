#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Stop orders", "[order filling][stop]")
{
    OrderBook ob{};
    Order SB55{Order::makeStopBuy(5, 55)};
    Order SB60{Order::makeStopBuy(5, 60)};
    Order SLB60_55{Order::makeStopLimitBuy(5, 60, 55)};

    Order SS50{Order::makeStopSell(5, 50)};
    Order SLS45_50{Order::makeStopLimitSell(5, 45, 50)};

    SECTION("Placing stop orders")
    {
        auto actual1{ob.placeOrder(SB55)};
        auto actual2{ob.placeOrder(SLB60_55)};
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
            *SLB60_55.id,
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(SLB60_55.id),
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
        ob.placeOrder(SLB60_55);
        ob.placeOrder(SS50);
        ob.placeOrder(SLS45_50);

        auto [actSBM, actSSM] = ob.getStopMaps();

        // volume doesnt matter for stop maps
        stop_buy_map expSBM{
            {5500, {0, order_list{SB55, SLB60_55}}}
        };

        stop_sell_map expSSM{
            {5000, {0, order_list{SS50, SLS45_50}}}
        };

        REQUIRE(compareMaps(actSBM, expSBM));
        REQUIRE(compareMaps(actSSM, expSSM));

        id_map expIDM{
            {SB55.id, OrderLocation{55, expSBM.at(5500).orders.begin(), OrderLocation::STOP_BUY}},
            {SLB60_55.id, OrderLocation{55, ++expSBM.at(5500).orders.begin(), OrderLocation::STOP_BUY}},
            {SS50.id, OrderLocation{55, expSSM.at(5000).orders.begin(), OrderLocation::STOP_SELL}},
            {SLS45_50.id, OrderLocation{55, ++expSSM.at(5000).orders.begin(), OrderLocation::STOP_SELL}},
        };

        OrderBookState expState{
            bid_map(), ask_map(), expIDM, trade_list(),
            orders{SB55, SLB60_55, SS50, SLS45_50},
            -1, -1, -1, 0 // stop orders dont count towards ob volume
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Market price movements trigger stop buy orders correctly")
    {
        // stop buy order: fully filled
        ob.placeOrder(SB55);

        Order buy57{Order::makeLimitBuy(3, 57)};
        Order sell57{Order::makeLimitSell(8, 57)};

        // this moves market price to 57, triggering SB55
        // but sell57 still has 5 shares left which is filled by SB55
        ob.placeOrder(buy57);
        ob.placeOrder(sell57);

        REQUIRE(ob.getStopMaps().first.empty());

        Trade expTrade1{nullptr, buy57.id, sell57.id, 57, 3, utils::now(), Order::Side::SELL};
        Trade expTrade2{nullptr, SB55.id, sell57.id, 57, 5, utils::now(), Order::Side::BUY};

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2},
            orders{SB55, buy57, sell57},
            -1, -1, 57, 0
        };

        REQUIRE(checkOBState(ob, expState));

        ob.clear();

        // stop buy order: partially filled, remaining volume cancelled
        ob.placeOrder(SB55);

        Order sell57Small{Order::makeLimitSell(5, 57)};

        // MP moves to 57 and triggers SB55
        // but only 2/5 gets filled by sell57Small
        // and remaining 3 shares cancelled
        ob.placeOrder(buy57);
        ob.placeOrder(sell57Small);

        REQUIRE(ob.getStopMaps().first.empty());

        Trade expTrade3{nullptr, buy57.id, sell57Small.id, 57, 3, utils::now(), Order::Side::SELL};
        Trade expTrade4{nullptr, SB55.id, sell57Small.id, 57, 2, utils::now(), Order::Side::BUY};

        OrderBookState expState2{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade3, expTrade4},
            orders{SB55, buy57, sell57Small},
            -1, -1, 57, 0
        };

        REQUIRE(checkOBState(ob, expState2));
    }

    SECTION("Market price movements trigger stop sell orders correctly")
    {
        // stop sell order: fully filled
        ob.placeOrder(SS50);

        Order sell48{Order::makeLimitSell(3, 48)};
        Order buy48{Order::makeLimitBuy(8, 48)};

        // this moves market price to 48, triggering SS50
        // but buy48 still has 5 shares left which is filled by SS50
        ob.placeOrder(sell48);
        ob.placeOrder(buy48);

        REQUIRE(ob.getStopMaps().second.empty());

        Trade expTrade1{nullptr, buy48.id, sell48.id, 48, 3, utils::now(), Order::Side::BUY};
        Trade expTrade2{nullptr, buy48.id, SS50.id, 48, 5, utils::now(), Order::Side::SELL};

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2},
            orders{SS50, sell48, buy48},
            -1, -1, 48, 0
        };

        REQUIRE(checkOBState(ob, expState));

        ob.clear();

        // stop sell order: partially filled, remaining volume cancelled
        ob.placeOrder(SS50);

        Order buy48Small{Order::makeLimitBuy(5, 48)};

        // MP moves to 48 and triggers SS50
        // but only 2/5 gets filled by buy48Small
        // and remaining 3 shares cancelled
        ob.placeOrder(sell48);
        ob.placeOrder(buy48Small);

        REQUIRE(ob.getStopMaps().second.empty());

        Trade expTrade3{nullptr, buy48Small.id, sell48.id, 48, 3, utils::now(), Order::Side::BUY};
        Trade expTrade4{nullptr, buy48Small.id, SS50.id, 48, 2, utils::now(), Order::Side::SELL};

        OrderBookState expState2{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade3, expTrade4},
            orders{SS50, sell48, buy48Small},
            -1, -1, 48, 0
        };

        REQUIRE(checkOBState(ob, expState2));
    }

    SECTION("Market price movements trigger stop limit buy orders correctly")
    {
        // stop limit buy order: fully filled
        ob.placeOrder(SLB60_55);

        Order buy57{Order::makeLimitBuy(3, 57)};
        Order sell57{Order::makeLimitSell(8, 57)};

        // this moves market price to 57, triggering SLB60_55
        // but sell57 still has 5 shares left which is filled by SLB60_55
        ob.placeOrder(buy57);
        ob.placeOrder(sell57);

        REQUIRE(ob.getStopMaps().first.empty());

        Trade expTrade1{nullptr, buy57.id, sell57.id, 57, 3, utils::now(), Order::Side::SELL};
        Trade expTrade2{nullptr, SLB60_55.id, sell57.id, 57, 5, utils::now(), Order::Side::BUY};

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2},
            orders{SLB60_55, buy57, sell57},
            -1, -1, 57, 0
        };

        REQUIRE(checkOBState(ob, expState));

        ob.clear();

        // stop limit buy order: partially filled, remaining volume goes to bidMap
        ob.placeOrder(SLB60_55);

        Order sell57Small{Order::makeLimitSell(5, 57)};

        // MP moves to 57 and triggers SLB60_55
        // but only 2/5 gets filled by sell57Small
        // and remaining 3 shares stay at 60
        ob.placeOrder(buy57);
        ob.placeOrder(sell57Small);

        REQUIRE(ob.getStopMaps().first.empty());

        Trade expTrade3{nullptr, buy57.id, sell57Small.id, 57, 3, utils::now(), Order::Side::SELL};
        Trade expTrade4{nullptr, SLB60_55.id, sell57Small.id, 57, 2, utils::now(), Order::Side::BUY};

        SLB60_55.volume = 3; // manually change this to reflect orderbook
        bid_map expBM{
            {6000, PriceLevel{3, order_list{SLB60_55}}}
        };

        id_map expIDM{
            {SLB60_55.id, OrderLocation{60, expBM.at(6000).orders.begin(), OrderLocation::BID}}
        };

        OrderBookState expState2{
            expBM, ask_map(), expIDM,
            trade_list{expTrade3, expTrade4},
            orders{SLB60_55, buy57, sell57Small},
            60, -1, 57, 3
        };

        REQUIRE(checkOBState(ob, expState2));
    }

    SECTION("Market price movements trigger stop limit sell orders correctly")
    {
        // stop limit sell order: fully filled
        Order SLS45_50{Order::makeStopLimitSell(5, 45, 50)};
        ob.placeOrder(SLS45_50);

        Order sell48{Order::makeLimitSell(3, 48)};
        Order buy48{Order::makeLimitBuy(8, 48)};

        // this moves market price to 48, triggering SLS45_50
        // but buy48 still has 5 shares left which is filled by SS50
        ob.placeOrder(sell48);
        ob.placeOrder(buy48);

        REQUIRE(ob.getStopMaps().second.empty());

        Trade expTrade1{nullptr, buy48.id, sell48.id, 48, 3, utils::now(), Order::Side::BUY};
        Trade expTrade2{nullptr, buy48.id, SLS45_50.id, 48, 5, utils::now(), Order::Side::SELL};

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2},
            orders{SLS45_50, sell48, buy48},
            -1, -1, 48, 0
        };

        REQUIRE(checkOBState(ob, expState));

        ob.clear();

        // stop sell order: partially filled, remaining volume goes to askMap
        ob.placeOrder(SLS45_50);

        Order buy48Small{Order::makeLimitBuy(5, 48)};

        // MP moves to 48 and triggers SLS45_50
        // but only 2/5 gets filled by buy48Small
        // and remaining 3 shares stay at 45
        ob.placeOrder(sell48);
        ob.placeOrder(buy48Small);

        REQUIRE(ob.getStopMaps().second.empty());

        Trade expTrade3{nullptr, buy48Small.id, sell48.id, 48, 3, utils::now(), Order::Side::BUY};
        Trade expTrade4{nullptr, buy48Small.id, SLS45_50.id, 48, 2, utils::now(), Order::Side::SELL};

        SLS45_50.volume = 3;
        ask_map expAM{
            {4500, PriceLevel{3, order_list{SLS45_50}}}
        };

        id_map expIDM{
            {SLS45_50.id, OrderLocation{45, expAM.at(4500).orders.begin(), OrderLocation::ASK}}
        };

        OrderBookState expState2{
            bid_map(), expAM, expIDM,
            trade_list{expTrade3, expTrade4},
            orders{SLS45_50, sell48, buy48Small},
            -1, 45, 48, 3
        };

        REQUIRE(checkOBState(ob, expState2));
    }

    SECTION("Market price movements that don't trigger stop orders")
    {
        
    }

    SECTION("Misaligned stop orders trigger immediately")
    {

    }

}
