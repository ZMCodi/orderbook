#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Increase order volume", "[order manipulation][increase volume]")
{
    OrderBook ob{};
    Order buy50{Order::makeLimitBuy(5, 50)};
    Order buy50_2{Order::makeLimitBuy(3, 50)};

    Order sell50{Order::makeLimitSell(5, 50)};
    Order sell50_2{Order::makeLimitSell(3, 50)};

    const uuids::uuid* id;

    // increase volume cancels the order and creates new one at the back
    SECTION("Increase volume full limit buy")
    {
        ob.placeOrder(buy50);
        ob.placeOrder(buy50_2);
        auto actual{ob.modifyVolume(buy50.get_id(), 10)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
        REQUIRE(*id != *buy50.get_id()); // ensure its a new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Volume increased from 5 to 10. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.volume == 10);
        REQUIRE(newOrder.side == buy50.side);
        REQUIRE(newOrder.price == buy50.price);
        REQUIRE(newOrder.type == buy50.type);
        REQUIRE(newOrder.timestamp > buy50.timestamp);

        bid_map expBM{
            {5000, PriceLevel{13, order_list{buy50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expBM.at(5000).orders.begin(), OrderLocation::BID}},
            {buy50_2.get_id(), OrderLocation{50.0, expBM.at(5000).orders.begin(), OrderLocation::BID}},
        };

        OrderBookState expState{
            expBM, ask_map(), stop_map(), expIDM,
            trade_list(), orders{buy50, buy50_2, newOrder},
            50, -1, -1, 13
        };

        REQUIRE(checkOBState(ob, expState));

        // internally represented as a buy50 cancel and new order
        OrderAudit expAudit{
            buy50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Increase volume full limit sell")
    {
        ob.placeOrder(sell50);
        ob.placeOrder(sell50_2);
        auto actual{ob.modifyVolume(sell50.get_id(), 10)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
        REQUIRE(*id != *sell50.get_id()); // ensure its a new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Volume increased from 5 to 10. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.volume == 10);
        REQUIRE(newOrder.side == sell50.side);
        REQUIRE(newOrder.price == sell50.price);
        REQUIRE(newOrder.type == sell50.type);
        REQUIRE(newOrder.timestamp > sell50.timestamp);

        ask_map expAM{
            {5000, PriceLevel{13, order_list{sell50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expAM.at(5000).orders.begin(), OrderLocation::ASK}},
            {sell50_2.get_id(), OrderLocation{50.0, expAM.at(5000).orders.begin(), OrderLocation::ASK}},
        };

        OrderBookState expState{
            bid_map(), expAM, stop_map(), expIDM,
            trade_list(), orders{sell50, sell50_2, newOrder},
            -1, 50, -1, 13
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Increase volume partial limit buy")
    {
        ob.placeOrder(buy50);
        // this fills 3/5 of buy50's volume
        ob.placeOrder(sell50_2);
        Trade expTrade{nullptr, buy50.get_id(), sell50_2.get_id(), 50, 3, utils::now(), Order::Side::SELL};

        ob.placeOrder(buy50_2);

        auto actual{ob.modifyVolume(buy50.get_id(), 10)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
        REQUIRE(*id != *buy50.get_id()); // ensure its a new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Volume increased from 2 to 10. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.volume == 10);
        REQUIRE(newOrder.side == buy50.side);
        REQUIRE(newOrder.price == buy50.price);
        REQUIRE(newOrder.type == buy50.type);
        REQUIRE(newOrder.timestamp > buy50.timestamp);

        bid_map expBM{
            {5000, PriceLevel{13, order_list{buy50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expBM.at(5000).orders.begin(), OrderLocation::BID}},
            {buy50_2.get_id(), OrderLocation{50.0, expBM.at(5000).orders.begin(), OrderLocation::BID}},
        };

        OrderBookState expState{
            expBM, ask_map(), stop_map(), expIDM,
            trade_list{expTrade}, orders{buy50, sell50_2, buy50_2, newOrder},
            50, -1, 50, 13
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            buy50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Increase volume partial limit sell")
    {
        ob.placeOrder(sell50);
        // this fills 3/5 of sell50's volume
        ob.placeOrder(buy50_2);
        Trade expTrade{nullptr, buy50_2.get_id(), sell50.get_id(), 50, 3, utils::now(), Order::Side::BUY};

        ob.placeOrder(sell50_2);

        auto actual{ob.modifyVolume(sell50.get_id(), 10)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
        REQUIRE(*id != *sell50.get_id()); // ensure its a new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Volume increased from 2 to 10. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.volume == 10);
        REQUIRE(newOrder.side == sell50.side);
        REQUIRE(newOrder.price == sell50.price);
        REQUIRE(newOrder.type == sell50.type);
        REQUIRE(newOrder.timestamp > sell50.timestamp);

        ask_map expAM{
            {5000, PriceLevel{13, order_list{sell50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expAM.at(5000).orders.begin(), OrderLocation::ASK}},
            {sell50_2.get_id(), OrderLocation{50.0, expAM.at(5000).orders.begin(), OrderLocation::ASK}},
        };

        OrderBookState expState{
            bid_map(), expAM, stop_map(), expIDM,
            trade_list{expTrade}, orders{sell50, buy50_2, sell50_2, newOrder},
            -1, 50, 50, 13
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }
}
