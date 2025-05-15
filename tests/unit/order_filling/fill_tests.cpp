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

    SECTION("Fill limit buy order")
    {
        ob.placeOrder(sell50);
        auto actual{ob.placeOrder(buy50)};

        Trade expTrade{nullptr, buy50.get_id(), sell50.get_id(), 50, 5, time_point(), Order::Side::BUY};
        OrderResult expected{
            *buy50.get_id(),
            OrderResult::FILLED,
            trades{expTrade},
            nullptr,
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), 
            trade_list{expTrade}, orders{sell50, buy50},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Fill limit sell order")
    {
        ob.placeOrder(buy50);
        auto actual{ob.placeOrder(sell50)};

        Trade expTrade{nullptr, buy50.get_id(), sell50.get_id(), 50, 5, time_point(), Order::Side::SELL};
        OrderResult expected{
            *sell50.get_id(), 
            OrderResult::FILLED, 
            trades{expTrade}, 
            nullptr, 
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), 
            trade_list{expTrade}, orders{buy50, sell50},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Reject market order when there is not enough liquidity")
    {
        auto actual{ob.placeOrder(buyMarket)};

        OrderResult expected{
            *buyMarket.get_id(),
            OrderResult::REJECTED,
            trades(), 
            &buyMarket, 
            "Not enough liquidity"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), 
            trade_list(), orders{buyMarket}, // rejected orders are still stored
            -1, -1, -1, 0
        };

        REQUIRE(checkOBState(ob, expState));

        // test with rvalue
        ob.clear();
        auto actual2{ob.placeOrder(Order::makeMarketBuy(5))};

        OrderResult expected2{
            actual2.order_id,
            OrderResult::REJECTED,
            trades(), 
            nullptr, // nullptr here since there is no order to point to 
            "Not enough liquidity"
        };

        buyMarket.id = &actual2.order_id; // change manually to compare
        REQUIRE(actual2.equals_to(expected2));

        OrderBookState expState2{
            bid_map(), ask_map(), id_map(), 
            trade_list(), orders{buyMarket}, // cannot reuse expState since its a copy
            -1, -1, -1, 0                    // but we can reuse buyMarket since it has the same
        };                                   // members as the rvalue and we changed the id
        REQUIRE(checkOBState(ob, expState2));
    }

    SECTION("Fill market buy order")
    {
        ob.placeOrder(sell50);
        auto actual{ob.placeOrder(buyMarket)};

        Trade expTrade{nullptr, buyMarket.get_id(), sell50.get_id(), 50, 5, time_point(), Order::Side::BUY};
        OrderResult expected{
            *buyMarket.get_id(),
            OrderResult::FILLED,
            trades{expTrade},
            nullptr,
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), 
            trade_list{expTrade}, orders{sell50, buyMarket},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));

        // test with rvalue
        ob.clear();
        sell50.id = nullptr; // reset id to add again
        ob.placeOrder(sell50);
        auto actual2{ob.placeOrder(Order::makeMarketBuy(5))};

        Trade expTrade2{nullptr, &actual2.order_id, sell50.get_id(), 50, 5, time_point(), Order::Side::BUY};
        OrderResult expected2{
            actual2.order_id,
            OrderResult::FILLED,
            trades{expTrade2},
            nullptr,
            "Order filled"
        };
        buyMarket.id = &actual2.order_id;
        buyMarket.volume = 5; // reset for orderList
        REQUIRE(actual2.equals_to(expected2));

        OrderBookState expState2{
            bid_map(), ask_map(), id_map(), 
            trade_list{expTrade2}, orders{sell50, buyMarket},
            -1, -1, 50, 0
        };
        REQUIRE(checkOBState(ob, expState2));

    }

    SECTION("Fill market sell order")
    {
        ob.placeOrder(buy50);
        auto actual{ob.placeOrder(sellMarket)};

        Trade expTrade{nullptr, buy50.get_id(), sellMarket.get_id(), 50, 5, time_point(), Order::Side::SELL};
        OrderResult expected{
            *sellMarket.get_id(),
            OrderResult::FILLED,
            trades{expTrade},
            nullptr,
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), 
            trade_list{expTrade}, orders{buy50, sellMarket},
            -1, -1, 50, 0
        };

        REQUIRE(checkOBState(ob, expState));

        // test with rvalue
        ob.clear();
        buy50.id = nullptr; // reset id to add again
        ob.placeOrder(buy50);
        auto actual2{ob.placeOrder(Order::makeMarketSell(5))};

        Trade expTrade2{nullptr, buy50.get_id(), &actual2.order_id, 50, 5, time_point(), Order::Side::SELL};
        OrderResult expected2{
            actual2.order_id,
            OrderResult::FILLED,
            trades{expTrade2},
            nullptr,
            "Order filled"
        };
        sellMarket.id = &actual2.order_id;
        sellMarket.volume = 5; // reset for orderList
        REQUIRE(actual2.equals_to(expected2));

        OrderBookState expState2{
            bid_map(), ask_map(), id_map(), 
            trade_list{expTrade2}, orders{buy50, sellMarket},
            -1, -1, 50, 0
        };
        REQUIRE(checkOBState(ob, expState2));

    }
}
