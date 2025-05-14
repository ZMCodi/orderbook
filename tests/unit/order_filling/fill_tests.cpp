#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Order filling", "[order filling][filling]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 5, Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 3, Order::Type::LIMIT, 50};
    Order sell55{Order::Side::SELL, 5, Order::Type::LIMIT, 55};
    Order sellMarket{Order::Side::SELL, 5, Order::Type::MARKET};

    SECTION("Fill limit buy order")
    {
        ob.place_order(sell50);

        OrderResult expected{
            buy50.id,
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{"", buy50.id, sell50.id, 50, 3, time_point(), Order::Side::BUY}
            },
            nullptr,
            ""
        };

        auto actual{ob.place_order(buy50)};
        REQUIRE(actual.equals_to(expected));

    }

    SECTION("Fill limit sell order")
    {
        ob.place_order(buy50);

        OrderResult expected{
            sell50.id, 
            OrderResult::FILLED, 
            std::vector<Trade>{
                Trade{"", buy50.id, sell50.id, 50, 3, time_point(), Order::Side::SELL}
            }, 
            nullptr, 
            ""
        };

        auto actual{ob.place_order(sell50)};
        REQUIRE(actual.equals_to(expected));

    }

    SECTION("Reject market order when there is not enough liquidity")
    {
        OrderResult expected{
            buyMarket.id,
            OrderResult::REJECTED,
            std::vector<Trade>(), 
            &buyMarket, 
            "Not enough liquidity"
        };

        auto actual{ob.place_order(buyMarket)};
        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Fill market buy order")
    {
        ob.place_order(sell55);

        OrderResult expected{
            buyMarket.id,
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{"", buyMarket.id, sell55.id, 55, 5, time_point(), Order::Side::BUY}
            },
            nullptr,
            ""
        };

        auto actual{ob.place_order(buyMarket)};
        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Fill market sell order")
    {
        ob.place_order(buy45);

        OrderResult expected{
            sellMarket.id,
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{"", buy45.id, sellMarket.id, 55, 5, time_point(), Order::Side::SELL}
            },
            nullptr,
            ""
        };

        auto actual{ob.place_order(sellMarket)};
        REQUIRE(actual.equals_to(expected));
    }
}
