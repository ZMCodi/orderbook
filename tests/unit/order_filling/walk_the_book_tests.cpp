#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Walking the book", "[order filling][walking the book]")
{
    OrderBook ob{};

    Order buy50{Order::makeLimitBuy(2, 50)};
    Order buy51{Order::makeLimitBuy(2, 51)};
    Order buy52{Order::makeLimitBuy(2, 52)};
    Order buy53{Order::makeLimitBuy(2, 53)};
    Order buyBig53{Order::makeLimitBuy(8, 53)};
    Order buyMarket{Order::makeMarketBuy(8)};

    Order sell50{Order::makeLimitSell(2, 50)};
    Order sell51{Order::makeLimitSell(2, 51)};
    Order sell52{Order::makeLimitSell(2, 52)};
    Order sell53{Order::makeLimitSell(2, 53)};
    Order sellBig50{Order::makeLimitSell(8, 50)};
    Order sellMarket{Order::makeMarketSell(8)};

    std::vector<std::reference_wrapper<Order>> buys{
        buy50, buy51, buy52, buy53
    };

    std::vector<std::reference_wrapper<Order>> sells{
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

        Trade expTrade1{nullptr, buyBig53.get_id(), sell50.get_id(), 50, 2, utils::now(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyBig53.get_id(), sell51.get_id(), 51, 2, utils::now(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyBig53.get_id(), sell52.get_id(), 52, 2, utils::now(), Order::Side::BUY};
        Trade expTrade4{nullptr, buyBig53.get_id(), sell53.get_id(), 53, 2, utils::now(), Order::Side::BUY};

        OrderResult expected{
            *buyBig53.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3, expTrade4},
            nullptr,
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            orders{sell50, sell51, sell52, sell53, buyBig53},
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

        Trade expTrade1{nullptr, buy53.get_id(), sellBig50.get_id(), 53, 2, utils::now(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellBig50.get_id(), 52, 2, utils::now(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellBig50.get_id(), 51, 2, utils::now(), Order::Side::SELL};
        Trade expTrade4{nullptr, buy50.get_id(), sellBig50.get_id(), 50, 2, utils::now(), Order::Side::SELL};

        OrderResult expected{
            *sellBig50.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3, expTrade4},
            nullptr,
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            orders{buy50, buy51, buy52, buy53, sellBig50},
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

        Trade expTrade1{nullptr, buyBig53.get_id(), sell50.get_id(), 50, 2, utils::now(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyBig53.get_id(), sell51.get_id(), 51, 2, utils::now(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyBig53.get_id(), sell52.get_id(), 52, 2, utils::now(), Order::Side::BUY};

        OrderResult expected{
            *id,
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            &ob.getOrderByID(id),
            "Partially filled 6 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        buyBig53.volume = 2; // ob doesnt modify original order so we have to do it manually

        bid_map expBM{
            {5300, PriceLevel{2, order_list{buyBig53}}}
        };

        id_map expIDM{
            {id, OrderLocation{53, expBM.at(5300).orders.begin(), OrderLocation::BID}}
        };

        buyBig53.volume = 8; // reset for orderList
        OrderBookState expState{
            expBM, ask_map(), expIDM,
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{sell50, sell51, sell52, buyBig53},
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

        Trade expTrade1{nullptr, buy53.get_id(), sellBig50.get_id(), 53, 2, utils::now(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellBig50.get_id(), 52, 2, utils::now(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellBig50.get_id(), 51, 2, utils::now(), Order::Side::SELL};

        OrderResult expected{
            *id,
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            &ob.getOrderByID(id),
            "Partially filled 6 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(ob.getOrderByID(id).volume == 2);
        sellBig50.volume = 2;

        ask_map expAM{
            {5000, PriceLevel{2, order_list{sellBig50}}}
        };

        id_map expIDM{
            {id, OrderLocation{50, expAM.at(5000).orders.begin(), OrderLocation::ASK}}
        };

        sellBig50.volume = 8; // reset for orderList
        OrderBookState expState{
            bid_map(), expAM, expIDM,
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{buy53, buy52, buy51, sellBig50},
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

        Trade expTrade1{nullptr, buyMarket.get_id(), sell50.get_id(), 50, 2, utils::now(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyMarket.get_id(), sell51.get_id(), 51, 2, utils::now(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyMarket.get_id(), sell52.get_id(), 52, 2, utils::now(), Order::Side::BUY};
        Trade expTrade4{nullptr, buyMarket.get_id(), sell53.get_id(), 53, 2, utils::now(), Order::Side::BUY};

        OrderResult expected{
            *buyMarket.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3, expTrade4},
            nullptr,
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        buyMarket.volume = 8; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            orders{sell50, sell51, sell52, sell53, buyMarket},
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

        Trade expTrade1{nullptr, buy53.get_id(), sellMarket.get_id(), 53, 2, utils::now(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellMarket.get_id(), 52, 2, utils::now(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellMarket.get_id(), 51, 2, utils::now(), Order::Side::SELL};
        Trade expTrade4{nullptr, buy50.get_id(), sellMarket.get_id(), 50, 2, utils::now(), Order::Side::SELL};

        OrderResult expected{
            *sellMarket.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3, expTrade4},
            nullptr,
            "Order filled"
        };

        REQUIRE(actual.equals_to(expected));

        sellMarket.volume = 8; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3, expTrade4},
            orders{buy50, buy51, buy52, buy53, sellMarket},
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

        Trade expTrade1{nullptr, buyMarket.get_id(), sell50.get_id(), 50, 2, utils::now(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyMarket.get_id(), sell51.get_id(), 51, 2, utils::now(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyMarket.get_id(), sell52.get_id(), 52, 2, utils::now(), Order::Side::BUY};

        OrderResult expected{
            *buyMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            nullptr,
            "Partially filled 6 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(buyMarket.volume == 2);

        buyMarket.volume = 8; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{sell50, sell51, sell52, buyMarket},
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

        Trade expTrade1{nullptr, buy53.get_id(), sellMarket.get_id(), 53, 2, utils::now(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy52.get_id(), sellMarket.get_id(), 52, 2, utils::now(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy51.get_id(), sellMarket.get_id(), 51, 2, utils::now(), Order::Side::SELL};

        OrderResult expected{
            *sellMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            nullptr,
            "Partially filled 6 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(sellMarket.volume == 2);

        sellMarket.volume = 8; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{buy53, buy52, buy51, sellMarket},
            -1, -1, 51, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }
}
