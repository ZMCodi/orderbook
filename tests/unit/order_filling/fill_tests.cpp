#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Order filling", "[order filling][filling]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 5, Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 3, Order::Type::LIMIT, 50};
    Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sell55{Order::Side::SELL, 5, Order::Type::LIMIT, 55};
    Order sellMarket{Order::Side::SELL, 5, Order::Type::MARKET};

    SECTION("Fill limit buy order")
    {

    }

    SECTION("Fill limit sell order")
    {
        ob.place_order(buy50);

        OrderResult expected{
            sell50.get_id(), 
            OrderResult::FILLED, 
            std::vector<Trade>{
                Trade{"", buy50.get_id(), sell50.get_id(), 50, 3, time_point(), Order::Side::SELL}
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
            buyMarket.get_id(),
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
            buyMarket.get_id(),
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{"", buyMarket.get_id(), sell55.get_id(), 55, 5, time_point(), Order::Side::BUY}
            },
            &buyMarket,
            ""
        };
        ob.place_order(buyMarket);
    }

    SECTION("Fill market sell order")
    {

    }
}
