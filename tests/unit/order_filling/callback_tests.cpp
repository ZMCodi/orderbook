#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "test_helpers.h"

TEST_CASE("Callback function notification", "[order][callbacks]")
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
    std::vector<const uuids::uuid*> tradeIDs{};
    callback callbackFn = [&callback_count, &tradeIDs](const Trade& trade){
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
            buy53.get_id(),
            OrderResult::PLACED,
            trade_ptrs(),
            &ob.getOrderByID(buy53.get_id()),
            "Order placed with callback"
        };

        REQUIRE(actual.equals_to(expected));

        // test with rvalues
        auto actual2 {ob.placeOrder(Order::makeLimitBuy(5, 49), callbackFn)};
        OrderResult expected2{
            actual2.order_id, // hack here since we cant get the actual id
            OrderResult::PLACED,
            trade_ptrs(),
            actual2.remainingOrder,
            "Order placed with callback"
        };

        REQUIRE(actual2.equals_to(expected2));
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
}
