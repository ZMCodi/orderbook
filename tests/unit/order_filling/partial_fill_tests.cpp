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

    // dummy uid pointer to instantiate Trade
    uuids::uuid uid{uuid_generator()};
    const auto* ptr{&uid};

    SECTION("Partial fill limit buy")
    {
        ob.place_order(sell50);
        auto actual{ob.place_order(buy50)};

        OrderResult expected{
            buy50.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, buy50.get_id(), sell50.get_id(), 50, 3, time_point(), Order::Side::BUY}
            },
            &buy50,
            "Partially filled 3 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(buy50.volume == 2);
    }

    SECTION("Partial fill limit sell")
    {
        ob.place_order(buy55);
        auto actual{ob.place_order(sell55)};

        OrderResult expected{
            sell55.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, buy55.get_id(), sell55.get_id(), 55, 3, time_point(), Order::Side::SELL}
            },
            &sell55,
            "Partially filled 3 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(sell55.volume == 2);
    }

    SECTION("Partial fill market buy")
    {
        ob.place_order(sell50);
        auto actual{ob.place_order(buyMarket)};

        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, buyMarket.get_id(), sell50.get_id(), 50, 3, time_point(), Order::Side::BUY}
            },
            &buyMarket,
            "Partially filled 3 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(buyMarket.volume == 2);
    }

    SECTION("Partial fill market sell")
    {
        ob.place_order(buy55);
        auto actual{ob.place_order(sellMarket)};

        OrderResult expected{
            sellMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, buy55.get_id(), sellMarket.get_id(), 55, 3, time_point(), Order::Side::SELL}
            },
            &sellMarket,
            "Partially filled 3 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(sellMarket.volume == 2);
    }
}
