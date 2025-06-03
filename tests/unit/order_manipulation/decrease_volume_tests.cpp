#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Decrease order volume", "[order manipulation][decrease volume]")
{
    OrderBook ob{};
    Order buy50{Order::makeLimitBuy(5, 50)};
    Order buy50_2{Order::makeLimitBuy(3, 50)};

    Order sell50{Order::makeLimitSell(5, 50)};
    Order sell50_2{Order::makeLimitSell(3, 50)};

    const uuids::uuid* id;

    SECTION("Decreasing volume to zero or negative throws error")
    {
        ob.placeOrder(buy50);
        REQUIRE_THROWS(ob.modifyVolume(buy50.get_id(), -1));
        REQUIRE_THROWS(ob.modifyVolume(buy50.get_id(), 0));

        // check that order is unchanged
        REQUIRE(ob.getOrderByID(buy50.get_id()).volume == 5);
    }

    SECTION("Decreasing volume to same value does nothing")
    {
        ob.placeOrder(buy50);
        id = buy50.get_id();
        auto actual{ob.modifyVolume(id, 5)};

        // could also change all throwing cases to return REJECTED
        // but im too lazy. mayber later
        OrderResult expected{
            *id,
            OrderResult::REJECTED, // modification rejected
            trades(),
            &ob.getOrderByID(id),
            "Volume unchanged"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 5);
        REQUIRE(ob.getAuditList().empty());
    }

    // decreasing volume should maintain time priority
    SECTION("Decrease volume full limit buy")
    {
        ob.placeOrder(buy50);
        id = buy50.get_id();
        ob.placeOrder(buy50_2);
        auto actual{ob.modifyVolume(id, 2)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &ob.getOrderByID(id),
            "Volume decreased from 5 to 2"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        buy50.volume = 2; // ob doesnt change original object

        bid_map expBM{
            {5000, PriceLevel{5, order_list{buy50, buy50_2}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, expBM.at(5000).orders.begin(), OrderLocation::BID}},
            {buy50_2.get_id(), OrderLocation{50.0, ++expBM.at(5000).orders.begin(), OrderLocation::BID}},
        };

        buy50.volume = 5; // reset for orderList
        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list(), orders{buy50, buy50_2},
            50, -1, -1, 5
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), 3
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Decrease volume full limit sell")
    {
        ob.placeOrder(sell50);
        id = sell50.get_id();
        ob.placeOrder(sell50_2);
        auto actual{ob.modifyVolume(id, 2)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &ob.getOrderByID(id),
            "Volume decreased from 5 to 2"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        sell50.volume = 2; // ob doesnt change original object

        ask_map expAM{
            {5000, PriceLevel{5, order_list{sell50, sell50_2}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, expAM.at(5000).orders.begin(), OrderLocation::ASK}},
            {sell50_2.get_id(), OrderLocation{50.0, ++expAM.at(5000).orders.begin(), OrderLocation::ASK}},
        };

        sell50.volume = 5; // reset for orderList
        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list(), orders{sell50, sell50_2},
            -1, 50, -1, 5
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), 3
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Decrease volume partial limit buy")
    {
        ob.placeOrder(buy50);
        id = buy50.get_id();
        // this fills 3/5 of buy50's volume
        ob.placeOrder(sell50_2);
        Trade expTrade{nullptr, buy50.get_id(), sell50_2.get_id(), 50, 3, utils::now(), Order::Side::SELL};

        ob.placeOrder(buy50_2);
        auto actual{ob.modifyVolume(id, 1)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &ob.getOrderByID(id),
            "Volume decreased from 2 to 1"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 1);
        buy50.volume = 1; // ob doesnt change original object

        bid_map expBM{
            {5000, PriceLevel{4, order_list{buy50, buy50_2}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, expBM.at(5000).orders.begin(), OrderLocation::BID}},
            {buy50_2.get_id(), OrderLocation{50.0, ++expBM.at(5000).orders.begin(), OrderLocation::BID}},
        };

        buy50.volume = 5; // reset for orderList
        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list{expTrade}, orders{buy50, sell50_2, buy50_2},
            50, -1, 50, 4
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), 1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Decrease volume partial limit sell")
    {
        ob.placeOrder(sell50);
        id = sell50.get_id();
        // this fills 3/5 of sell50's volume
        ob.placeOrder(buy50_2);
        Trade expTrade{nullptr, buy50_2.get_id(), sell50.get_id(), 50, 3, utils::now(), Order::Side::BUY};

        ob.placeOrder(sell50_2);
        auto actual{ob.modifyVolume(id, 1)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &ob.getOrderByID(id),
            "Volume decreased from 2 to 1"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 1);
        sell50.volume = 1;

        ask_map expAM{
            {5000, PriceLevel{4, order_list{sell50, sell50_2}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, expAM.at(5000).orders.begin(), OrderLocation::ASK}},
            {sell50_2.get_id(), OrderLocation{50.0, ++expAM.at(5000).orders.begin(), OrderLocation::ASK}},
        };

        sell50.volume = 5; // reset for orderList
        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list{expTrade}, orders{sell50, buy50_2, sell50_2},
            -1, 50, 50, 4
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), 1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }
}
