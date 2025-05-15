#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Walking the book", "[order filling][walking the book]")
{
    OrderBook ob{};

    Order buy50{Order::Side::BUY, 2, Order::Type::LIMIT, 50};
    Order buy51{Order::Side::BUY, 2, Order::Type::LIMIT, 51};
    Order buy52{Order::Side::BUY, 2, Order::Type::LIMIT, 52};
    Order buy53{Order::Side::BUY, 2, Order::Type::LIMIT, 53};
    Order buyBig53{Order::Side::BUY, 8, Order::Type::LIMIT, 53};
    Order buyMarket{Order::Side::BUY, 8, Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 2, Order::Type::LIMIT, 50};
    Order sell51{Order::Side::SELL, 2, Order::Type::LIMIT, 51};
    Order sell52{Order::Side::SELL, 2, Order::Type::LIMIT, 52};
    Order sell53{Order::Side::SELL, 2, Order::Type::LIMIT, 53};
    Order sellBig50{Order::Side::SELL, 8, Order::Type::LIMIT, 50};
    Order sellMarket{Order::Side::SELL, 8, Order::Type::MARKET};

    std::vector<Order> buys{
        buy50, buy51, buy52, buy53
    };

    std::vector<Order> sells{
        sell50, sell51, sell52, sell53
    };

    const uuids::uuid* id;

    SECTION("Walk the book limit buy")
    {
        for (auto sell : sells)
        {
            ob.placeOrder(sell);
        }
        auto actual{ob.placeOrder(buyBig53)};

        Trade expTrade1{nullptr, buyBig53.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyBig53.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyBig53.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY};
        Trade expTrade4{nullptr, buyBig53.get_id(), sell53.get_id(), 53, 2, time_point(), Order::Side::BUY};

        OrderResult expected{
            buyBig53.get_id(),
            OrderResult::FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3, &expTrade4},
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            -1, -1, 53, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Walk the book limit sell")
    {
        for (auto buy : buys)
        {
            ob.placeOrder(buy);
        }
        auto actual{ob.placeOrder(sellBig50)};

        Trade expTrade1{nullptr, buy53.get_id(), sellBig50.get_id(), 53, 2, time_point(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellBig50.get_id(), 52, 2, time_point(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellBig50.get_id(), 51, 2, time_point(), Order::Side::SELL};
        Trade expTrade4{nullptr, buy50.get_id(), sellBig50.get_id(), 50, 2, time_point(), Order::Side::SELL};

        OrderResult expected{
            sellBig50.get_id(),
            OrderResult::FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3, &expTrade4},
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            -1, -1, 50, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Walk the book limit buy partial fill")
    {
        ob.placeOrder(sell50);
        ob.placeOrder(sell51);
        ob.placeOrder(sell52);
        auto actual{ob.placeOrder(buyBig53)};
        id = buyBig53.get_id();

        Trade expTrade1{nullptr, buyBig53.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyBig53.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyBig53.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY};

        OrderResult expected{
            id,
            OrderResult::PARTIALLY_FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3},
            &ob.getOrderByID(id),
            "Partially filled 6 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        buyBig53.volume = 2; // ob doesnt modify original order so we have to do it manually

        bid_map expBM{
            {53.0, PriceLevel{2, order_list{buyBig53}}}
        };

        id_map expIDM{
            {id, OrderLocation{53, expBM.at(53.0).orders.begin(), Order::Side::BUY}}
        };

        OrderBookState expState{
            expBM, ask_map(), expIDM, trade_list{expTrade1, expTrade2, expTrade3},
            53, -1, 52, 2
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Walk the book limit sell partial fill")
    {
        ob.placeOrder(buy53);
        ob.placeOrder(buy52);
        ob.placeOrder(buy51);
        auto actual{ob.placeOrder(sellBig50)};
        id = sellBig50.get_id();

        Trade expTrade1{nullptr, buy53.get_id(), sellBig50.get_id(), 53, 2, time_point(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellBig50.get_id(), 52, 2, time_point(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellBig50.get_id(), 51, 2, time_point(), Order::Side::SELL};

        OrderResult expected{
            id,
            OrderResult::PARTIALLY_FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3},
            &ob.getOrderByID(id),
            "Partially filled 6 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        sellBig50.volume = 2;

        ask_map expAM{
            {50.0, PriceLevel{2, order_list{sellBig50}}}
        };

        id_map expIDM{
            {id, OrderLocation{50, expAM.at(50.0).orders.begin(), Order::Side::SELL}}
        };

        OrderBookState expState{
            bid_map(), expAM, expIDM, trade_list{expTrade1, expTrade2, expTrade3},
            -1, 50, 51, 2
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Walk the book market buy")
    {
        for (auto sell : sells)
        {
            ob.placeOrder(sell);
        }
        auto actual{ob.placeOrder(buyMarket)};

        Trade expTrade1{nullptr, buyMarket.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyMarket.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyMarket.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY};
        Trade expTrade4{nullptr, buyMarket.get_id(), sell53.get_id(), 53, 2, time_point(), Order::Side::BUY};

        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3, &expTrade4},
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            -1, -1, 53, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Walk the book market sell")
    {
        for (auto buy : buys)
        {
            ob.placeOrder(buy);
        }
        auto actual{ob.placeOrder(sellMarket)};

        Trade expTrade1{nullptr, buy53.get_id(), sellMarket.get_id(), 53, 2, time_point(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellMarket.get_id(), 52, 2, time_point(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellMarket.get_id(), 51, 2, time_point(), Order::Side::SELL};
        Trade expTrade4{nullptr, buy50.get_id(), sellMarket.get_id(), 50, 2, time_point(), Order::Side::SELL};

        OrderResult expected{
            sellMarket.get_id(),
            OrderResult::FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3, &expTrade4},
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            -1, -1, 50, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Walk the book market buy partial fill")
    {
        ob.placeOrder(sell50);
        ob.placeOrder(sell51);
        ob.placeOrder(sell52);
        auto actual{ob.placeOrder(buyMarket)};

        Trade expTrade1{nullptr, buyMarket.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyMarket.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyMarket.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY};

        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3},
            &buyMarket,
            "Partially filled 6 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(buyMarket.volume == 2);

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade1, expTrade2, expTrade3},
            -1, -1, 52, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Walk the book market sell partial fill")
    {
        ob.placeOrder(buy53);
        ob.placeOrder(buy52);
        ob.placeOrder(buy51);
        auto actual{ob.placeOrder(sellMarket)};

        Trade expTrade1{nullptr, buy53.get_id(), sellMarket.get_id(), 53, 2, time_point(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellMarket.get_id(), 52, 2, time_point(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellMarket.get_id(), 51, 2, time_point(), Order::Side::SELL};

        OrderResult expected{
            sellMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            trade_ptrs{&expTrade1, &expTrade2, &expTrade3},
            &sellMarket,
            "Partially filled 6 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(sellMarket.volume == 2);

        OrderBookState expState{
            bid_map(), ask_map(), id_map(), trade_list{expTrade1, expTrade2, expTrade3},
            -1, -1, 51, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }
}
