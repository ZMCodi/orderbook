#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Order filling", "[order filling]")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 5, Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 5, Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 5, Order::Type::LIMIT, 50};
    Order sell55{Order::Side::SELL, 5, Order::Type::LIMIT, 55};
    Order sellMarket{Order::Side::SELL, 5, Order::Type::MARKET};

    // dummy uid pointer to instantiate Trade
    uuids::uuid uid{uuid_generator()};
    const auto* ptr{&uid};

    SECTION("Fill limit buy order")
    {
        ob.place_order(sell50);
        auto actual{ob.place_order(buy50)};

        Trade expTrade{ptr, buy50.get_id(), sell50.get_id(), 50, 5, time_point(), Order::Side::BUY};
        OrderResult expected{
            buy50.get_id(),
            OrderResult::FILLED,
            trade_ptrs{&expTrade},
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Fill limit sell order")
    {
        ob.place_order(buy50);
        auto actual{ob.place_order(sell50)};

        Trade expTrade{ptr, buy50.get_id(), sell50.get_id(), 50, 5, time_point(), Order::Side::SELL};
        OrderResult expected{
            sell50.get_id(), 
            OrderResult::FILLED, 
            trade_ptrs{&expTrade}, 
            nullptr, 
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Reject market order when there is not enough liquidity")
    {
        auto actual{ob.place_order(buyMarket)};

        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::REJECTED,
            trade_ptrs(), 
            &buyMarket, 
            "Not enough liquidity"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list(),
            -1, -1, -1, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Fill market buy order")
    {
        ob.place_order(sell50);
        auto actual{ob.place_order(buyMarket)};

        Trade expTrade{ptr, buyMarket.get_id(), sell50.get_id(), 50, 5, time_point(), Order::Side::BUY};
        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::FILLED,
            trade_ptrs{&expTrade},
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Fill market sell order")
    {
        ob.place_order(buy50);
        auto actual{ob.place_order(sellMarket)};

        Trade expTrade{ptr, buy50.get_id(), sellMarket.get_id(), 50, 5, time_point(), Order::Side::SELL};
        OrderResult expected{
            sellMarket.get_id(),
            OrderResult::FILLED,
            trade_ptrs{&expTrade},
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }
}
