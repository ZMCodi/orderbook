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
            buy50.id,
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buy50.id, sell50.id, 50, 3, time_point(), Order::Side::BUY}
            },
            &buy50,
            "Partially filled 3 shares, 2 shares remaining"
        };

        auto actual{ob.place_order(buy50)};
        REQUIRE(actual.equals_to(expected));
        REQUIRE(buy50.volume == 2);
    }

    SECTION("Partial fill limit sell")
    {
        ob.place_order(buy55);

        OrderResult expected{
            sell55.id,
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buy55.id, sell55.id, 55, 3, time_point(), Order::Side::SELL}
            },
            &sell55,
            "Partially filled 3 shares, 2 shares remaining"
        };

        auto actual{ob.place_order(sell55)};
        REQUIRE(actual.equals_to(expected));
        REQUIRE(sell55.volume == 2);
    }

    SECTION("Partial fill market buy")
    {
        ob.place_order(sell50);

        OrderResult expected{
            buyMarket.id,
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buyMarket.id, sell50.id, 50, 3, time_point(), Order::Side::BUY}
            },
            &buyMarket,
            "Partially filled 3 shares, remaining order cancelled"
        };

        auto actual{ob.place_order(buyMarket)};
        REQUIRE(actual.equals_to(expected));
        REQUIRE(buyMarket.volume == 2);
    }

    SECTION("Partial fill market sell")
    {
        ob.place_order(buy55);

        OrderResult expected{
            sellMarket.id,
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{"", buy55.id, sellMarket.id, 55, 3, time_point(), Order::Side::SELL}
            },
            &sellMarket,
            "Partially filled 3 shares, remaining order cancelled"
        };

        auto actual{ob.place_order(sellMarket)};
        REQUIRE(actual.equals_to(expected));
        REQUIRE(sellMarket.volume == 2);
    }
}
