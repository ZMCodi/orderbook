#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Check time priority", "[order filling][time priority checking]")
{
    OrderBook ob{};

    Order buy50_1{Order::Side::BUY, 5, Order::Type::LIMIT, 50};
    Order buy50_2{Order::Side::BUY, 10, Order::Type::LIMIT, 50};
    Order buy50_3{Order::Side::BUY, 2, Order::Type::LIMIT, 50};
    Order buy51{Order::Side::BUY, 17, Order::Type::LIMIT, 51};
    Order buy51Small{Order::Side::BUY, 10, Order::Type::LIMIT, 51};
    Order buyMarket{Order::Side::BUY, 17, Order::Type::MARKET};
    Order buyMarketSmall{Order::Side::BUY, 10, Order::Type::MARKET};

    Order sell51_1{Order::Side::SELL, 5, Order::Type::LIMIT, 51};
    Order sell51_2{Order::Side::SELL, 10, Order::Type::LIMIT, 51};
    Order sell51_3{Order::Side::SELL, 2, Order::Type::LIMIT, 51};
    Order sell50{Order::Side::SELL, 17, Order::Type::LIMIT, 50};
    Order sell50Small{Order::Side::SELL, 10, Order::Type::LIMIT, 50};
    Order sellMarket{Order::Side::SELL, 17, Order::Type::MARKET};
    Order sellMarketSmall{Order::Side::SELL, 10, Order::Type::MARKET};

    SECTION("Time priority for buy")
    {
        ob.placeOrder(buy50_1);
        ob.placeOrder(buy50_2);
        ob.placeOrder(buy50_3);

        REQUIRE(buy50_1.timestamp < buy50_2.timestamp);
        REQUIRE(buy50_2.timestamp < buy50_3.timestamp);
        REQUIRE(compareOrderLists(ob.bidsAt(50.0), order_list{buy50_1, buy50_2, buy50_3}));
    }

    SECTION("Time priority for sell")
    {
        ob.placeOrder(sell51_1);
        ob.placeOrder(sell51_2);
        ob.placeOrder(sell51_3);

        REQUIRE(sell51_1.timestamp < sell51_2.timestamp);
        REQUIRE(sell51_2.timestamp < sell51_3.timestamp);
        REQUIRE(compareOrderLists(ob.asksAt(51.0), order_list{sell51_1, sell51_2, sell51_3}));
    }

    SECTION("Time priority for mixed orders")
    {
        ob.placeOrder(buy50_1);
        ob.placeOrder(sell51_1);
        ob.placeOrder(buy50_2);
        ob.placeOrder(sell51_2);
        ob.placeOrder(buy50_3);
        ob.placeOrder(sell51_3);

        REQUIRE(buy50_1.timestamp < sell51_1.timestamp);
        REQUIRE(sell51_1.timestamp < buy50_2.timestamp);
        REQUIRE(buy50_2.timestamp < sell51_2.timestamp);
        REQUIRE(sell51_2.timestamp < buy50_3.timestamp);
        REQUIRE(buy50_3.timestamp < sell51_3.timestamp);
        REQUIRE(compareOrderLists(ob.bidsAt(50.0), order_list{buy50_1, buy50_2, buy50_3}));
        REQUIRE(compareOrderLists(ob.asksAt(51.0), order_list{sell51_1, sell51_2, sell51_3}));
    }

    SECTION("Buy limit orders fill based on time priority")
    {
        ob.placeOrder(sell51_1);
        ob.placeOrder(sell51_2);
        ob.placeOrder(sell51_3);
        auto actual{ob.placeOrder(buy51)};

        Trade expTrade1{nullptr, buy51.get_id(), sell51_1.get_id(), 51, 5, time_point(), Order::Side::BUY};
        Trade expTrade2{nullptr, buy51.get_id(), sell51_2.get_id(), 51, 10, time_point(), Order::Side::BUY};
        Trade expTrade3{nullptr, buy51.get_id(), sell51_3.get_id(), 51, 2, time_point(), Order::Side::BUY};

        OrderResult expected{
            *buy51.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            nullptr,
            "Order filled"
        };
        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{sell51_1, sell51_2, sell51_3, buy51},
            -1, -1, 51, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Sell limit orders fill based on time priority")
    {
        ob.placeOrder(buy50_1);
        ob.placeOrder(buy50_2);
        ob.placeOrder(buy50_3);
        auto actual{ob.placeOrder(sell50)};

        Trade expTrade1{nullptr, buy50_1.get_id(), sell50.get_id(), 50, 5, time_point(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy50_2.get_id(), sell50.get_id(), 50, 10, time_point(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy50_3.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::SELL};

        OrderResult expected{
            *sell50.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            nullptr,
            "Order filled"
        };
        REQUIRE(actual.equals_to(expected));

        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{buy50_1, buy50_2, buy50_3, sell50},
            -1, -1, 50, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Buy market orders fill based on time priority")
    {
        ob.placeOrder(sell51_1);
        ob.placeOrder(sell51_2);
        ob.placeOrder(sell51_3);
        auto actual{ob.placeOrder(buyMarket)};

        Trade expTrade1{nullptr, buyMarket.get_id(), sell51_1.get_id(), 51, 5, time_point(), Order::Side::BUY};
        Trade expTrade2{nullptr, buyMarket.get_id(), sell51_2.get_id(), 51, 10, time_point(), Order::Side::BUY};
        Trade expTrade3{nullptr, buyMarket.get_id(), sell51_3.get_id(), 51, 2, time_point(), Order::Side::BUY};

        OrderResult expected{
            *buyMarket.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            nullptr,
            "Order filled"
        };
        REQUIRE(actual.equals_to(expected));

        buyMarket.volume = 17; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{sell51_1, sell51_2, sell51_3, buyMarket},
            -1, -1, 51, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Sell market orders fill based on time priority")
    {
        ob.placeOrder(buy50_1);
        ob.placeOrder(buy50_2);
        ob.placeOrder(buy50_3);
        auto actual{ob.placeOrder(sellMarket)};

        Trade expTrade1{nullptr, buy50_1.get_id(), sellMarket.get_id(), 50, 5, time_point(), Order::Side::SELL};
        Trade expTrade2{nullptr, buy50_2.get_id(), sellMarket.get_id(), 50, 10, time_point(), Order::Side::SELL};
        Trade expTrade3{nullptr, buy50_3.get_id(), sellMarket.get_id(), 50, 2, time_point(), Order::Side::SELL};

        OrderResult expected{
            *sellMarket.get_id(),
            OrderResult::FILLED,
            trades{expTrade1, expTrade2, expTrade3},
            nullptr,
            "Order filled"
        };
        REQUIRE(actual.equals_to(expected));

        sellMarket.volume = 17; // reset for orderList
        OrderBookState expState{
            bid_map(), ask_map(), id_map(),
            trade_list{expTrade1, expTrade2, expTrade3},
            orders{buy50_1, buy50_2, buy50_3, sellMarket},
            -1, -1, 50, 0
        };
        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Time priority is maintained after limit buy partial fill")
    {
        ob.placeOrder(sell51_1);
        ob.placeOrder(sell51_2);
        ob.placeOrder(sell51_3);
        ob.placeOrder(buy51Small);

        // sell51_1 is filled, sell51_2 is partially filled
        REQUIRE(compareOrderLists(ob.asksAt(51.0), order_list{sell51_2, sell51_3}));
        REQUIRE(sell51_2.volume == 5);
        REQUIRE(sell51_3.volume == 2);
    }

    SECTION("Time priority is maintained after limit sell partial fill")
    {
        ob.placeOrder(buy50_1);
        ob.placeOrder(buy50_2);
        ob.placeOrder(buy50_3);
        ob.placeOrder(sell50Small);

        // buy50_1 is filled, buy50_2 is partially filled
        REQUIRE(compareOrderLists(ob.bidsAt(50.0), order_list{buy50_2, buy50_3}));
        REQUIRE(buy50_2.volume == 5);
        REQUIRE(buy50_3.volume == 2);
    }

    SECTION("Time priority is maintained after market buy partial fill")
    {
        ob.placeOrder(sell51_1);
        ob.placeOrder(sell51_2);
        ob.placeOrder(sell51_3);
        ob.placeOrder(buyMarketSmall);

        // sell51_1 is filled, sell51_2 is partially filled
        REQUIRE(compareOrderLists(ob.asksAt(51.0), order_list{sell51_2, sell51_3}));
        REQUIRE(sell51_2.volume == 5);
        REQUIRE(sell51_3.volume == 2);
    }

    SECTION("Time priority is maintained after market sell partial fill")
    {
        ob.placeOrder(buy50_1);
        ob.placeOrder(buy50_2);
        ob.placeOrder(buy50_3);
        ob.placeOrder(sellMarketSmall);

        // buy50_1 is filled, buy50_2 is partially filled
        REQUIRE(compareOrderLists(ob.bidsAt(50.0), order_list{buy50_2, buy50_3}));
        REQUIRE(buy50_2.volume == 5);
        REQUIRE(buy50_3.volume == 2);
    }

    SECTION("Cancelling orders maintain time priority")
    {
        ob.placeOrder(buy50_1);
        ob.placeOrder(buy50_2);
        ob.placeOrder(buy50_3);
        ob.placeOrder(sell51_1);
        ob.placeOrder(sell51_2);
        ob.placeOrder(sell51_3);

        ob.cancelOrder(buy50_2.get_id());
        ob.cancelOrder(sell51_2.get_id());

        REQUIRE(compareOrderLists(ob.bidsAt(50.0), order_list{buy50_1, buy50_3}));
        REQUIRE(compareOrderLists(ob.asksAt(51.0), order_list{sell51_1, sell51_3}));
    }
}
