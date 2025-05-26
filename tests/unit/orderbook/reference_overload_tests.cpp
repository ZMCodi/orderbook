#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("UUID reference overloads", "[orderbook][uuid reference]")
{
    OrderBook ob{};

    Order buy50{Order::Side::BUY, 5, Order::Type::LIMIT, 50};
    Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};

    // initialize uuids
    ob.placeOrder(buy50);
    ob.placeOrder(sell60);

    // we can get away by only testing with copies since
    // using actual references is a stricter requirement
    uuids::uuid buy_id_copy{*buy50.get_id()};
    uuids::uuid sell_id_copy{*sell60.get_id()};

    SECTION("getOrderByID with reference")
    {
        const Order& buy_ref{ob.getOrderByID(buy_id_copy)};
        const Order& sell_ref{ob.getOrderByID(sell_id_copy)};

        REQUIRE(buy_ref.equals_to(buy50));
        REQUIRE(sell_ref.equals_to(sell60));
    }

    SECTION("cancelOrder with reference")
    {
        auto result{ob.cancelOrder(buy_id_copy)};

        REQUIRE(result.status == OrderResult::CANCELLED);
        REQUIRE(result.order_id == buy_id_copy);
        REQUIRE(ob.bidsAt(50.0).empty());
    }

    SECTION("modifyVolume with reference")
    {
        auto result{ob.modifyVolume(buy_id_copy, 3)};

        REQUIRE(result.status == OrderResult::MODIFIED);
        REQUIRE(result.order_id == buy_id_copy);
        REQUIRE(ob.getOrderByID(buy50.get_id()).volume == 3);
    }

    SECTION("modifyPrice with reference")
    {
        auto result{ob.modifyPrice(buy_id_copy, 45)};

        // Verify the price was modified
        REQUIRE(result.status == OrderResult::MODIFIED);
        REQUIRE(ob.bidsAt(50.0).empty());
        REQUIRE(!ob.bidsAt(45.0).empty());
    }

    SECTION("registerCallback with reference")
    {
        int callback_count{};
        callback callbackFn = [&callback_count](TradeCopy) { ++callback_count; };

        REQUIRE(ob.registerCallback(buy_id_copy, callbackFn));

        // this triggers the callback
        ob.placeOrder(Order::makeLimitSell(5, 50));
        REQUIRE(callback_count == 1);
    }

    SECTION("removeCallback with reference")
    {
        int callback_count{};
        callback callbackFn = [&callback_count](TradeCopy) { ++callback_count; };

        ob.registerCallback(buy50.get_id(), callbackFn);
        REQUIRE(ob.removeCallback(buy_id_copy));

        // check callback is not triggered after a match
        ob.placeOrder(Order::makeLimitSell(5, 50));
        REQUIRE(callback_count == 0);
    }

    SECTION("UUID not in pool")
    {
        // create a UUID that doesn't exist in the pool
        auto fakeID{utils::uuid_generator()};

        // Verify operations with non-existent UUID throw exceptions
        REQUIRE_THROWS(ob.getOrderByID(fakeID));
        REQUIRE_THROWS(ob.cancelOrder(fakeID));
        REQUIRE_THROWS(ob.modifyVolume(fakeID, 10));
        REQUIRE_THROWS(ob.modifyPrice(fakeID, 45));
        REQUIRE_FALSE(ob.registerCallback(fakeID, [](TradeCopy){}));
        REQUIRE_FALSE(ob.removeCallback(fakeID));
    }
}
