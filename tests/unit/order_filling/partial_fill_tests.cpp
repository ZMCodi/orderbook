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

    const uuids::uuid* id;

    SECTION("Partial fill limit buy")
    {
        ob.placeOrder(sell50);
        auto actual{ob.placeOrder(buy50)};
        id = buy50.get_id();

        Trade expTrade{nullptr, id, sell50.get_id(), 50, 3, time_point(), Order::Side::BUY};
        OrderResult expected{
            *id,
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade},
            &ob.getOrderByID(id),
            "Partially filled 3 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        buy50.volume = 2; // ob doesnt change the original object so we have to do it manually

        bid_map expBM{
            {5000, PriceLevel{2, order_list{buy50}}}
        };

        id_map expIDM{
            {id, OrderLocation{50, expBM.at(5000).orders.begin(), Order::Side::BUY}}
        };

        buy50.volume = 5; // reset for orderList
        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list{expTrade}, orders{sell50, buy50},
            50, -1, 50, 2
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Partial fill limit sell")
    {
        ob.placeOrder(buy55);
        auto actual{ob.placeOrder(sell55)};
        id = sell55.get_id();

        Trade expTrade{nullptr, buy55.get_id(), sell55.get_id(), 55, 3, time_point(), Order::Side::SELL};
        OrderResult expected{
            *id,
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade},
            &ob.getOrderByID(id),
            "Partially filled 3 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        sell55.volume = 2;

        ask_map expAM{
            {5500, PriceLevel{2, order_list{sell55}}}
        };

        id_map expIDM{
            {id, OrderLocation{55, expAM.at(5500).orders.begin(), Order::Side::SELL}}
        };

        sell55.volume = 5; // reset for orderList
        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list{expTrade}, orders{buy55, sell55},
            -1, 55, 55, 2
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Partial fill market buy")
    {
        ob.placeOrder(sell50);
        auto actual{ob.placeOrder(buyMarket)};

        Trade expTrade{nullptr, buyMarket.get_id(), sell50.get_id(), 50, 3, time_point(), Order::Side::BUY};
        OrderResult expected{
            *buyMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade},
            nullptr,
            "Partially filled 3 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(buyMarket.volume == 2); // ob modifies in place for market orders

        buyMarket.volume = 5; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade}, orders{sell50, buyMarket},
            -1, -1, 50, 0
        };
        REQUIRE(checkOBState(ob, expState));

        // test with rvalue
        ob.clear();
        ob.placeOrder(sell50);
        auto actual2{ob.placeOrder(Order::makeMarketBuy(5))};

        Trade expTrade2{nullptr, &actual2.order_id, sell50.get_id(), 50, 3, time_point(), Order::Side::BUY};
        OrderResult expected2{
            actual2.order_id,
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade2},
            nullptr,
            "Partially filled 3 shares, remaining order cancelled"
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

    SECTION("Partial fill market sell")
    {
        ob.placeOrder(buy55);
        auto actual{ob.placeOrder(sellMarket)};

        Trade expTrade{nullptr, buy55.get_id(), sellMarket.get_id(), 55, 3, time_point(), Order::Side::SELL};
        OrderResult expected{
            *sellMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade},
            nullptr,
            "Partially filled 3 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(sellMarket.volume == 2);

        sellMarket.volume = 5; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade}, orders{buy55, sellMarket},
            -1, -1, 55, 0
        };
        REQUIRE(checkOBState(ob, expState));

        // test with rvalue
        ob.clear();
        ob.placeOrder(buy55);
        auto actual2{ob.placeOrder(Order::makeMarketSell(5))};

        Trade expTrade2{nullptr, &actual2.order_id, buy55.get_id(), 55, 3, time_point(), Order::Side::SELL};
        OrderResult expected2{
            actual2.order_id,
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade2},
            nullptr,
            "Partially filled 3 shares, remaining order cancelled"
        };
        sellMarket.id = &actual2.order_id;
        sellMarket.volume = 5; // reset for orderList
        REQUIRE(actual2.equals_to(expected2));

        OrderBookState expState2{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade2}, orders{buy55, sellMarket},
            -1, -1, 55, 0
        };
        REQUIRE(checkOBState(ob, expState2));
    }
}
