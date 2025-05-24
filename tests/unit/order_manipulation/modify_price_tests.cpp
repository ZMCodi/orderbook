#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Price modification", "[order manipulation][price modification]")
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

    // price modification is implemented as a cancel and new order
    SECTION("Decreasing price to negative throws error")
    {
        ob.placeOrder(sell50);
        REQUIRE_THROWS(ob.modifyPrice(sell50.get_id(), -1));
    }

    SECTION("Modify price full limit buy")
    {
        ob.placeOrder(buy50);
        ob.placeOrder(buy45);
        auto actual{ob.modifyPrice(buy50.get_id(), 45)};
        id = &actual.order_id;
        REQUIRE(*id != *buy50.get_id()); // ensure new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Price changed from 50 to 45. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.price == Catch::Approx(45.0));
        REQUIRE(newOrder.side == buy50.side);
        REQUIRE(newOrder.volume == buy50.volume);
        REQUIRE(newOrder.type == buy50.type);
        REQUIRE(newOrder.timestamp > buy50.timestamp);

        bid_map expBM{
            {45.0, PriceLevel{10, order_list{buy45, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{45.0, ++expBM.at(45.0).orders.begin(), Order::Side::BUY}},
            {buy45.get_id(), OrderLocation{45.0, expBM.at(45.0).orders.begin(), Order::Side::BUY}},
        };

        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list(), orders{buy50, buy45, newOrder},
            45, -1, -1, 10
        };

        REQUIRE(checkOBState(ob, expState));

        // internally represented as a buy50 cancel and new order
        OrderAudit expAudit{
            buy50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Modify price full limit sell")
    {
        ob.placeOrder(sell50);
        ob.placeOrder(sell60);
        auto actual{ob.modifyPrice(sell50.get_id(), 60)};
        id = &actual.order_id;
        REQUIRE(*id != *sell50.get_id()); // ensure new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Price changed from 50 to 60. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.price == Catch::Approx(60.0));
        REQUIRE(newOrder.side == sell50.side);
        REQUIRE(newOrder.volume == sell50.volume);
        REQUIRE(newOrder.type == sell50.type);
        REQUIRE(newOrder.timestamp > sell50.timestamp);

        ask_map expAM{
            {60.0, PriceLevel{15, order_list{sell60, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{60.0, ++expAM.at(60.0).orders.begin(), Order::Side::SELL}},
            {sell60.get_id(), OrderLocation{60.0, expAM.at(60.0).orders.begin(), Order::Side::SELL}},
        };

        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list(), orders{sell50, sell60, newOrder},
            -1, 60, -1, 15
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Modify price partial limit buy")
    {
        ob.placeOrder(buy50);
        // this fills 3/5 of buy50's volume
        ob.placeOrder(sell50_2);
        Trade expTrade{nullptr, buy50.get_id(), sell50_2.get_id(), 50, 3, time_point(), Order::Side::SELL};

        ob.placeOrder(buy45);
        auto actual{ob.modifyPrice(buy50.get_id(), 45)};
        id = &actual.order_id;
        REQUIRE(*id != *buy50.get_id()); // ensure new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Price changed from 50 to 45. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.price == Catch::Approx(45.0));
        REQUIRE(newOrder.side == buy50.side);
        REQUIRE(newOrder.volume == 2);
        REQUIRE(newOrder.type == buy50.type);
        REQUIRE(newOrder.timestamp > buy50.timestamp);

        bid_map expBM{
            {45.0, PriceLevel{7, order_list{buy45, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{45.0, ++expBM.at(45.0).orders.begin(), Order::Side::BUY}},
            {buy45.get_id(), OrderLocation{45.0, expBM.at(45.0).orders.begin(), Order::Side::BUY}},
        };

        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list{expTrade}, orders{buy50, sell50_2, buy45, newOrder},
            45, -1, 50, 7
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            buy50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Modify price partial limit sell")
    {
        ob.placeOrder(sell50);
        // this fills 3/5 of sell50's volume
        ob.placeOrder(buy50_2);
        Trade expTrade{nullptr, buy50_2.get_id(), sell50.get_id(), 50, 3, time_point(), Order::Side::BUY};

        ob.placeOrder(sell60);
        auto actual{ob.modifyPrice(sell50.get_id(), 60)};
        id = &actual.order_id;
        REQUIRE(*id != *sell50.get_id()); // ensure new order
        const Order& newOrder{ob.getOrderByID(id)};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades(),
            &newOrder,
            "Price changed from 50 to 60. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        // check fields are copied properly
        REQUIRE(newOrder.price == Catch::Approx(60.0));
        REQUIRE(newOrder.side == sell50.side);
        REQUIRE(newOrder.volume == 2);
        REQUIRE(newOrder.type == sell50.type);
        REQUIRE(newOrder.timestamp > sell50.timestamp);

        ask_map expAM{
            {60.0, PriceLevel{12, order_list{sell60, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{60.0, ++expAM.at(60.0).orders.begin(), Order::Side::SELL}},
            {sell60.get_id(), OrderLocation{60.0, expAM.at(60.0).orders.begin(), Order::Side::SELL}},
        };

        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list{expTrade}, orders{sell50, buy50_2, sell60, newOrder},
            -1, 60, 50, 12
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), time_point(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }
}
