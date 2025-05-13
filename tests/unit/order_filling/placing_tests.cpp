#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Placing orders", "[order filling][placing]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 5, Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 3, Order::Type::LIMIT, 50};
    Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sell55{Order::Side::SELL, 5, Order::Type::LIMIT, 55};
    Order sellMarket{Order::Side::SELL, 5, Order::Type::MARKET};

    SECTION("Place buy order")
    {
        OrderResult expected{
            buy50.get_id(), 
            OrderResult::PLACED, 
            std::vector<Trade>(), 
            &buy50, 
            ""
        };

        auto actual{ob.place_order(buy50)};
        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Place sell order")
    {
        OrderResult expected{
            sell50.get_id(), 
            OrderResult::PLACED, 
            std::vector<Trade>(), 
            &sell50, 
            ""
        };

        auto actual{ob.place_order(sell50)};
        REQUIRE(actual.equals_to(expected));
    }
}
