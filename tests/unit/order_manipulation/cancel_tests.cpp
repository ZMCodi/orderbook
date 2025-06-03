#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Order cancellation", "[order manipulation][cancellation]")
{
    OrderBook ob{};
    Order buy50{Order::makeLimitBuy(5, 50)};
    Order buy50_2{Order::makeLimitBuy(3, 50)};

    Order sell50{Order::makeLimitSell(5, 50)};
    Order sell50_2{Order::makeLimitSell(3, 50)};

    const uuids::uuid* id;

    SECTION("Modifications only work on orders in the book")
    {
        // nonexistent order
        auto fakeID(utils::uuid_generator());
        REQUIRE_THROWS(ob.cancelOrder(&fakeID));
        REQUIRE_THROWS(ob.cancelOrder(nullptr));
        REQUIRE_THROWS(ob.modifyPrice(&fakeID, 50));
        REQUIRE_THROWS(ob.modifyPrice(nullptr, 50));
        REQUIRE_THROWS(ob.modifyVolume(&fakeID, 50));
        REQUIRE_THROWS(ob.modifyVolume(nullptr, 50));

        // order exists but not in the book
        ob.placeOrder(buy50);
        ob.placeOrder(sell50); // this will fill buy50 and remove it from the book
        id = buy50.get_id();

        REQUIRE_THROWS(ob.cancelOrder(id));
        REQUIRE_THROWS(ob.modifyPrice(id, 50));
        REQUIRE_THROWS(ob.modifyVolume(id, 50));

        // order exists and in the book
        ob.placeOrder(buy50_2);
        id = buy50_2.get_id();
        REQUIRE_NOTHROW(ob.cancelOrder(id));

        ob.clear();
        ob.placeOrder(buy50_2);
        id = buy50_2.get_id();
        REQUIRE_NOTHROW(ob.modifyPrice(id, 50));

        ob.clear();
        ob.placeOrder(buy50_2);
        id = buy50_2.get_id();
        REQUIRE_NOTHROW(ob.modifyVolume(id, 50));
    }

    SECTION("Cancel full limit buy")
    {
        ob.placeOrder(buy50);
        id = buy50.get_id();
        auto actual{ob.cancelOrder(id)};

        OrderResult expected{
            *id,
            OrderResult::CANCELLED,
            trades(),
            nullptr,
            "Order cancelled with 5 unfilled shares"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list(), orders{buy50},
            -1, -1, -1, 0
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Cancel full limit sell")
    {
        ob.placeOrder(sell50);
        id = sell50.get_id();
        auto actual{ob.cancelOrder(id)};

        OrderResult expected{
            *id,
            OrderResult::CANCELLED,
            trades(),
            nullptr,
            "Order cancelled with 5 unfilled shares"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list(), orders{sell50},
            -1, -1, -1, 0
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Cancel partial limit buy")
    {
        ob.placeOrder(buy50);
        id = buy50.get_id();
        ob.placeOrder(sell50_2); // this will partially fill buy50
        auto actual{ob.cancelOrder(id)};

        OrderResult expected{
            *id,
            OrderResult::CANCELLED,
            trades(),
            nullptr,
            "Order cancelled with 2 unfilled shares"
        };

        REQUIRE(actual.equals_to(expected));

        Trade expTrade{nullptr, id, sell50_2.get_id(), 50, 3, utils::now(), Order::Side::SELL};
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade}, orders{buy50, sell50_2},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Cancel partial limit sell")
    {
        ob.placeOrder(sell50);
        id = sell50.get_id();
        ob.placeOrder(buy50_2);
        auto actual{ob.cancelOrder(id)};

        OrderResult expected{
            *id,
            OrderResult::CANCELLED,
            trades(),
            nullptr,
            "Order cancelled with 2 unfilled shares"
        };

        REQUIRE(actual.equals_to(expected));

        Trade expTrade{nullptr, buy50_2.get_id(), id, 50, 3, utils::now(), Order::Side::BUY};
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade}, orders{sell50, buy50_2},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));

        OrderAudit expAudit{
            id, utils::now(), -1
        };
        REQUIRE(ob.getAuditList().size() == 1);
        REQUIRE(ob.getAuditList()[0].equals_to(expAudit));
    }

    SECTION("Cancel stop orders")
    {

    }
}
