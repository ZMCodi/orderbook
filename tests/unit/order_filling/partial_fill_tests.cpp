#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Partial filling orders", "[order filling][partial filling]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 5, Order::Type::LIMIT, 50};
    Order buy55{Order::Side::BUY, 3, Order::Type::LIMIT, 55};
    Order buyMarket{Order::Side::BUY, 5, Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 3, Order::Type::LIMIT, 50};
    Order sell55{Order::Side::SELL, 5, Order::Type::LIMIT, 55};
    Order sellMarket{Order::Side::SELL, 5, Order::Type::MARKET};

    SECTION("Partial fill limit buy")
    {
        ob.place_order(sell50);

        OrderResult expected{
            buy50.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buy50.get_id(), sell50.get_id(), 50, 3, time_point(), Order::Side::BUY}
            },
            &modify_volume(buy50, -3),
            "Partially filled 3 shares, 2 shares remaining"
        };

        // add volume back
        modify_volume(buy50, 3);

        auto actual{ob.place_order(buy50)};
        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Partial fill limit sell")
    {
        ob.place_order(buy55);

        OrderResult expected{
            sell55.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buy55.get_id(), sell55.get_id(), 55, 3, time_point(), Order::Side::SELL}
            },
            &modify_volume(sell55, -3),
            "Partially filled 3 shares, 2 shares remaining"
        };

        // add volume back
        modify_volume(sell55, 3);

        auto actual{ob.place_order(sell55)};
        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Partial fill market buy")
    {
        ob.place_order(sell50);

        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buyMarket.get_id(), sell50.get_id(), 50, 3, time_point(), Order::Side::BUY}
            },
            &modify_volume(buyMarket, -3),
            "Partially filled 3 shares, remaining order cancelled"
        };

        // add the volume back
        modify_volume(buyMarket, 3);

        auto actual{ob.place_order(buyMarket)};
        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Partial fill market sell")
    {
        // execute trades until liquidity is exhausted
        // remaining order is cancelled
        ob.place_order(buy55);

        OrderResult expected{
            sellMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buy55.get_id(), sellMarket.get_id(), 55, 3, time_point(), Order::Side::SELL}
            },
            &modify_volume(sellMarket, -3),
            "Partially filled 3 shares, remaining order cancelled"
        };

        // add volume back
        modify_volume(sellMarket, 3);

        auto actual{ob.place_order(sellMarket)};
        REQUIRE(actual.equals_to(expected));
    }
}
