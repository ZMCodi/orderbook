#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "test_helpers.h"

TEST_CASE("Callback function notification", "[order filling][callbacks]")
{
    OrderBook ob{};

    Order buy50{Order::Side::BUY, 2, Order::Type::LIMIT, 50};
    Order buy51{Order::Side::BUY, 2, Order::Type::LIMIT, 51};
    Order buy52{Order::Side::BUY, 2, Order::Type::LIMIT, 52};
    Order buy53{Order::Side::BUY, 2, Order::Type::LIMIT, 53};
    Order buyBig53{Order::Side::BUY, 8, Order::Type::LIMIT, 53};
    Order buyMarket{Order::Side::BUY, 8, Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 2, Order::Type::LIMIT, 50};
    Order sell51{Order::Side::SELL, 2, Order::Type::LIMIT, 51};
    Order sell52{Order::Side::SELL, 2, Order::Type::LIMIT, 52};
    Order sell53{Order::Side::SELL, 2, Order::Type::LIMIT, 53};
    Order sellBig50{Order::Side::SELL, 8, Order::Type::LIMIT, 50};
    Order sellMarket{Order::Side::SELL, 8, Order::Type::MARKET};

    int callback_count{};
    std::vector<uuids::uuid> tradeIDs{};
    callback callbackFn = [&callback_count, &tradeIDs](TradeCopy trade){
        ++callback_count;
        tradeIDs.push_back(trade.get_id());
    };

    SECTION("Registering callbacks")
    {
        ob.placeOrder(buy51);
        // orders staying in the book can register a callback
        REQUIRE(ob.registerCallback(buy51.get_id(), callbackFn));

        ob.placeOrder(sell51); // this is filled immediately
        // filled orders cannot register a callback
        REQUIRE(!ob.registerCallback(sell51.get_id(), callbackFn));

        // rejected orders cannot register a callback
        ob.placeOrder(buyMarket); // rejected because no liquidity
        REQUIRE(!ob.registerCallback(buyMarket.get_id(), callbackFn));

        // cancelled orders cannot register a callback
        ob.placeOrder(sell50);
        ob.cancelOrder(sell50.get_id());
        REQUIRE(!ob.registerCallback(sell50.get_id(), callbackFn));

        // callback can be registered upon placing order
        auto actual{ob.placeOrder(buy53, callbackFn)};
        OrderResult expected{
            *buy53.get_id(),
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(buy53.get_id()),
            "Order placed"
        };

        REQUIRE(actual.equals_to(expected));

        // test with rvalues
        auto actual2 {ob.placeOrder(Order::makeLimitBuy(5, 49), callbackFn)};
        OrderResult expected2{
            actual2.order_id, // hack here since we cant get the actual id
            OrderResult::PLACED,
            trades(),
            actual2.remainingOrder,
            "Order placed"
        };

        REQUIRE(actual2.equals_to(expected2));

        // nonexistent orders
        auto fakeID(utils::uuid_generator());
        REQUIRE(!ob.registerCallback(&fakeID, callbackFn));
        REQUIRE(!ob.registerCallback(nullptr, callbackFn));
    }

    SECTION("Removing callbacks")
    {
        ob.placeOrder(buy51, callbackFn);
        // orders staying in the book can remove a callback
        REQUIRE(ob.removeCallback(buy51.get_id()));

        ob.placeOrder(sell51, callbackFn); // this is filled immediately
        // filled orders cannot remove a callback
        REQUIRE(!ob.removeCallback(sell51.get_id()));

        // rejected orders cannot remove a callback
        ob.placeOrder(buyMarket, callbackFn); // rejected because no liquidity
        REQUIRE(!ob.removeCallback(buyMarket.get_id()));

        // cancelled orders cannot remove a callback
        ob.placeOrder(sell50, callbackFn);
        ob.cancelOrder(sell50.get_id());
        REQUIRE(!ob.removeCallback(sell50.get_id()));

        // orders with no callback can remove a callback (idempotency)
        ob.placeOrder(buy50);
        REQUIRE(ob.removeCallback(buy50.get_id()));

        // nonexistent orders
        auto fakeID(utils::uuid_generator());
        REQUIRE(!ob.removeCallback(&fakeID));
        REQUIRE(!ob.removeCallback(nullptr));
    }

    SECTION("Unmatched orders don't trigger callbacks")
    {
        ob.placeOrder(buy50, callbackFn);
        ob.placeOrder(buy51, callbackFn);
        ob.placeOrder(sell52, callbackFn);
        ob.placeOrder(sell53, callbackFn);

        REQUIRE(callback_count == 0);
        REQUIRE(tradeIDs.empty());
    }

    SECTION("Matched orders trigger callbacks")
    {
        ob.placeOrder(buy50, callbackFn);
        auto id{ob.placeOrder(sell50).trades.at(0).get_id()};

        REQUIRE(callback_count == 1);
        REQUIRE(tradeIDs[0] == id);

        ob.placeOrder(buy51, callbackFn);
        auto id2{ob.placeOrder(sell51, callbackFn).trades.at(0).get_id()};
        // this will trigger the callback twice by buy51 and sell51

        REQUIRE(callback_count == 3);
        REQUIRE(tradeIDs[1] == id2);
        REQUIRE(tradeIDs[2] == id2);

        // for completeness, lets test two different callbacks
        int buy_count{};
        ob.placeOrder(buy52, [&buy_count](TradeCopy){++buy_count;});
        int sell_count{};
        ob.placeOrder(sell52, [&sell_count](TradeCopy){++sell_count;});
        REQUIRE(buy_count == 1);
        REQUIRE(sell_count == 1);
    }

    SECTION("Multiple matches trigger callbacks once every match")
    {
        ob.placeOrder(buyBig53, callbackFn);
        auto id1{ob.placeOrder(sell50).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 1);
        REQUIRE(tradeIDs[0] == id1);

        auto id2{ob.placeOrder(sell51).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 2);
        REQUIRE(tradeIDs[1] == id2);

        auto id3{ob.placeOrder(sell52).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 3);
        REQUIRE(tradeIDs[2] == id3);

        auto id4{ob.placeOrder(sell53).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 4);
        REQUIRE(tradeIDs[3] == id4);

        // also works the other way around
        ob.placeOrder(buy50);
        ob.placeOrder(buy51);
        ob.placeOrder(buy52);
        ob.placeOrder(buy53);

        // this triggers 4 matches
        auto ids{ob.placeOrder(sellMarket, callbackFn).trades};
        REQUIRE(callback_count == 8);
        REQUIRE(tradeIDs[4] == ids[0].get_id());
        REQUIRE(tradeIDs[5] == ids[1].get_id());
        REQUIRE(tradeIDs[6] == ids[2].get_id());
        REQUIRE(tradeIDs[7] == ids[3].get_id());
    }

    SECTION("Cancelling/Replacing callbacks")
    {
        // check it works
        ob.placeOrder(buyBig53, callbackFn);
        auto id1{ob.placeOrder(sell50).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 1);
        REQUIRE(tradeIDs[0] == id1);

        // remove it and match
        ob.removeCallback(buyBig53.get_id());
        auto id2{ob.placeOrder(sell51).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 1);
        REQUIRE(tradeIDs.size() == 1);
        REQUIRE(tradeIDs[0] != id2);

        // re-add it
        ob.registerCallback(buyBig53.get_id(), callbackFn);
        auto id3{ob.placeOrder(sell52).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 2);
        REQUIRE(tradeIDs[1] == id3);

        // change to another callback
        ob.registerCallback(buyBig53.get_id(), [&callback_count, &tradeIDs](TradeCopy trade){
            callback_count += 5;
            tradeIDs[0] = trade.get_id();
        });
        auto id4{ob.placeOrder(sell53).trades.at(0).get_id()}; // fills 2
        REQUIRE(callback_count == 7);
        REQUIRE(tradeIDs[0] == id4);
        REQUIRE(tradeIDs.size() == 2); // old callback is not called
    }

    SECTION("Modifying order retains callbacks")
    {
        // decrease volume
        ob.placeOrder(buyBig53, callbackFn);
        ob.modifyVolume(buyBig53.get_id(), 2);
        auto id1{ob.placeOrder(sell51).trades.at(0).get_id()};
        REQUIRE(callback_count == 1);
        REQUIRE(tradeIDs[0] == id1);

        // increase volume
        ob.placeOrder(sell53, callbackFn);
        ob.modifyVolume(sell53.get_id(), 8);
        auto id2{ob.placeOrder(buyBig53).trades.at(0).get_id()};
        REQUIRE(callback_count == 2);
        REQUIRE(tradeIDs[1] == id2);

        // change price
        ob.placeOrder(buy50, callbackFn);
        ob.modifyPrice(buy50.get_id(), 52);
        auto id3{ob.placeOrder(sell52).trades.at(0).get_id()};
        REQUIRE(callback_count == 3);
        REQUIRE(tradeIDs[2] == id3);
    }
}
