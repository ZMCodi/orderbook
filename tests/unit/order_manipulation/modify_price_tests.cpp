#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Price modification", "[order manipulation][price modification]")
{
    OrderBook ob{};
    Order buy50{Order::makeLimitBuy(5, 50)};
    Order buy50_2{Order::makeLimitBuy(3, 50)};
    Order buy45{Order::makeLimitBuy(5, 45)};
    Order buyMarket{Order::makeMarketBuy(10)};

    Order sell50{Order::makeLimitSell(5, 50)};
    Order sell50_2{Order::makeLimitSell(3, 50)};
    Order sell60{Order::makeLimitSell(10, 60)};
    Order sellMarket{Order::makeMarketSell(5)};

    const uuids::uuid* id;

    // price modification is implemented as a cancel and new order
    SECTION("Decreasing price to negative throws error")
    {
        ob.placeOrder(sell50);
        REQUIRE_THROWS(ob.modifyPrice(sell50.get_id(), -1));
    }

    SECTION("Changing price to same value does nothing")
    {
        ob.placeOrder(buy50);
        id = buy50.get_id();
        auto actual{ob.modifyPrice(id, 50)};

        OrderResult expected{
            *id,
            OrderResult::REJECTED, // modification rejected
            trades(),
            &ob.getOrderByID(id),
            "Price unchanged"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).price == 50);
        REQUIRE(ob.getAuditList().empty());
    }

    SECTION("Modify price full limit buy")
    {
        ob.placeOrder(buy50);
        ob.placeOrder(buy45);
        auto actual{ob.modifyPrice(buy50.get_id(), 45)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
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
            {4500, PriceLevel{10, order_list{buy45, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{45.0, ++expBM.at(4500).orders.begin(), OrderLocation::BID}},
            {buy45.get_id(), OrderLocation{45.0, expBM.at(4500).orders.begin(), OrderLocation::BID}},
        };

        OrderBookState expState{
            expBM, ask_map(), stop_map(), expIDM,
            trade_list(), orders{buy50, buy45, newOrder},
            45, -1, -1, 10
        };

        REQUIRE(checkOBState(ob, expState));

        // internally represented as a buy50 cancel and new order
        OrderAudit expAudit{
            buy50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Modify price full limit sell")
    {
        ob.placeOrder(sell50);
        ob.placeOrder(sell60);
        auto actual{ob.modifyPrice(sell50.get_id(), 60)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
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
            {6000, PriceLevel{15, order_list{sell60, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{60.0, ++expAM.at(6000).orders.begin(), OrderLocation::ASK}},
            {sell60.get_id(), OrderLocation{60.0, expAM.at(6000).orders.begin(), OrderLocation::ASK}},
        };

        OrderBookState expState{
            bid_map(), expAM, stop_map(), expIDM,
            trade_list(), orders{sell50, sell60, newOrder},
            -1, 60, -1, 15
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Modify price partial limit buy")
    {
        ob.placeOrder(buy50);
        // this fills 3/5 of buy50's volume
        ob.placeOrder(sell50_2);
        Trade expTrade{nullptr, buy50.get_id(), sell50_2.get_id(), 50, 3, utils::now(), Order::Side::SELL};

        ob.placeOrder(buy45);
        auto actual{ob.modifyPrice(buy50.get_id(), 45)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
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
            {4500, PriceLevel{7, order_list{buy45, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{45.0, ++expBM.at(4500).orders.begin(), OrderLocation::BID}},
            {buy45.get_id(), OrderLocation{45.0, expBM.at(4500).orders.begin(), OrderLocation::BID}},
        };

        OrderBookState expState{
            expBM, ask_map(), stop_map(), expIDM,
            trade_list{expTrade}, orders{buy50, sell50_2, buy45, newOrder},
            45, -1, 50, 7
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            buy50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Modify price partial limit sell")
    {
        ob.placeOrder(sell50);
        // this fills 3/5 of sell50's volume
        ob.placeOrder(buy50_2);
        Trade expTrade{nullptr, buy50_2.get_id(), sell50.get_id(), 50, 3, utils::now(), Order::Side::BUY};

        ob.placeOrder(sell60);
        auto actual{ob.modifyPrice(sell50.get_id(), 60)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
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
            {6000, PriceLevel{12, order_list{sell60, newOrder}}}
        };

        id_map expIDM{
            {id, OrderLocation{60.0, ++expAM.at(6000).orders.begin(), OrderLocation::ASK}},
            {sell60.get_id(), OrderLocation{60.0, expAM.at(6000).orders.begin(), OrderLocation::ASK}},
        };

        OrderBookState expState{
            bid_map(), expAM, stop_map(), expIDM,
            trade_list{expTrade}, orders{sell50, buy50_2, sell60, newOrder},
            -1, 60, 50, 12
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell50.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Changing price to a better price can trigger matches")
    {
        // no matches here
        ob.placeOrder(buy50);
        ob.placeOrder(buy45);
        ob.placeOrder(sell60);

        // this causes sell60 to match buy50 and buy45
        auto actual{ob.modifyPrice(sell60.get_id(), 45)};
        auto it{ob.getIDPool().find(actual.order_id)};

        id = &(*it);
        Order newOrder{Order::makeLimitSell(10, 45)};
        newOrder.id = id;

        Trade expTrade1{nullptr, buy50.get_id(), id, 50, 5, utils::now(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy45.get_id(), id, 45, 5, utils::now(), Order::Side::SELL};

        OrderResult expected{
            *id,
            OrderResult::MODIFIED,
            trades{expTrade1, expTrade2},
            nullptr,
            "Price changed from 60 to 45. New ID generated."
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), stop_map(), id_map(),
            trade_list{expTrade1, expTrade2}, orders{buy50, buy45, sell60, newOrder},
            -1, -1, 45, 0
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            sell60.get_id(), utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }
}
