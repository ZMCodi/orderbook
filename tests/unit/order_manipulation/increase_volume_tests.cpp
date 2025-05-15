#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Increase order volume", "[order manipulation][increase volume]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 5,  Order::Type::LIMIT, 50};
    Order buy50_2{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};

    Order sell50{Order::Side::SELL, 5,  Order::Type::LIMIT, 50};
    Order sell50_2{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};

    const uuids::uuid* id;

    // increase volume cancels the order and creates new one at the back
    SECTION("Increase volume full limit buy")
    {
        ob.placeOrder(buy50);
        ob.placeOrder(buy50_2);
        auto actual{ob.modifyVolume(buy50.get_id(), 10)};
        id = &actual.order_id;
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
            {50.0, PriceLevel{13, order_list{buy50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expBM.at(50.0).orders.begin(), Order::Side::BUY}},
            {buy50_2.get_id(), OrderLocation{50.0, expBM.at(50.0).orders.begin(), Order::Side::BUY}},
        };

        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list(), orders{buy50, buy50_2, newOrder},
            50, -1, -1, 13
        };

        REQUIRE(checkOBState(ob, expState));

        // internally represented as a buy50 cancel and new order
        OrderAudit expAudit{
            buy50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Increase volume full limit sell")
    {
        ob.placeOrder(sell50);
        ob.placeOrder(sell50_2);
        auto actual{ob.modifyVolume(sell50.get_id(), 10)};
        id = &actual.order_id;
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
            {50.0, PriceLevel{13, order_list{sell50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expAM.at(50.0).orders.begin(), Order::Side::SELL}},
            {sell50_2.get_id(), OrderLocation{50.0, expAM.at(50.0).orders.begin(), Order::Side::SELL}},
        };

        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list(), orders{sell50, sell50_2, newOrder},
            50, -1, -1, 13
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Increase volume partial limit buy")
    {
        ob.placeOrder(buy50);
        // this fills 3/5 of buy50's volume
        Trade expTrade{ob.placeOrder(sell50_2).trades[0]};
        ob.placeOrder(buy50_2);
        auto actual{ob.modifyVolume(buy50.get_id(), 10)};
        id = &actual.order_id;
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
            {50.0, PriceLevel{13, order_list{buy50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expBM.at(50.0).orders.begin(), Order::Side::BUY}},
            {buy50_2.get_id(), OrderLocation{50.0, expBM.at(50.0).orders.begin(), Order::Side::BUY}},
        };

        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list{expTrade}, orders{buy50, sell50_2, buy50_2, newOrder},
            50, -1, 50, 13
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            buy50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Increase volume partial limit sell")
    {
        ob.placeOrder(sell50);
        // this fills 3/5 of sell50's volume
        Trade expTrade{ob.placeOrder(buy50_2).trades[0]};
        ob.placeOrder(sell50_2);
        auto actual{ob.modifyVolume(sell50.get_id(), 10)};
        id = &actual.order_id;
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
            {50.0, PriceLevel{13, order_list{sell50_2, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{50.0, ++expAM.at(50.0).orders.begin(), Order::Side::SELL}},
            {sell50_2.get_id(), OrderLocation{50.0, expAM.at(50.0).orders.begin(), Order::Side::SELL}},
        };

        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list{expTrade}, orders{sell50, buy50_2, sell50_2, newOrder},
            -1, 50, 50, 13
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }
}
