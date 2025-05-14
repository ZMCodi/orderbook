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

    // dummy uid pointer to instantiate Trade
    uuids::uuid uid{uuid_generator()};
    const auto* ptr{&uid};

    SECTION("Walk the book limit buy")
    {
        for (auto sell : sells)
        {
            ob.place_order(sell);
        }
        auto actual{ob.place_order(buyBig53)};

        OrderResult expected{
            buyBig53.get_id(),
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{ptr, buyBig53.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyBig53.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyBig53.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyBig53.get_id(), sell53.get_id(), 53, 2, time_point(), Order::Side::BUY}
            },
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Walk the book limit sell")
    {
        for (auto buy : buys)
        {
            ob.place_order(buy);
        }
        auto actual{ob.place_order(sellBig50)};

        OrderResult expected{
            sellBig50.get_id(),
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{ptr, sellBig50.get_id(), buy53.get_id(), 53, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellBig50.get_id(), buy52.get_id(), 52, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellBig50.get_id(), buy51.get_id(), 51, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellBig50.get_id(), buy50.get_id(), 50, 2, time_point(), Order::Side::SELL}
            },
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Walk the book limit buy partial fill")
    {
        ob.place_order(sell50);
        ob.place_order(sell51);
        ob.place_order(sell52);
        auto actual{ob.place_order(buyBig53)};

        OrderResult expected{
            buyBig53.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, buyBig53.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyBig53.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyBig53.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY}
            },
            &buyBig53,
            "Partially filled 6 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(buyBig53.volume == 2);
    }

    SECTION("Walk the book limit sell partial fill")
    {
        ob.place_order(buy53);
        ob.place_order(buy52);
        ob.place_order(buy51);
        auto actual{ob.place_order(sellBig50)};

        OrderResult expected{
            sellBig50.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, sellBig50.get_id(), buy53.get_id(), 53, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellBig50.get_id(), buy52.get_id(), 52, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellBig50.get_id(), buy51.get_id(), 51, 2, time_point(), Order::Side::SELL}            },
            &sellBig50,
            "Partially filled 6 shares, 2 shares remaining"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(sellBig50.volume == 2);
    }

    SECTION("Walk the book market buy")
    {
        for (auto sell : sells)
        {
            ob.place_order(sell);
        }
        auto actual{ob.place_order(buyMarket)};

        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{ptr, buyMarket.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyMarket.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyMarket.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyMarket.get_id(), sell53.get_id(), 53, 2, time_point(), Order::Side::BUY}
            },
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Walk the book market sell")
    {
        for (auto buy : buys)
        {
            ob.place_order(buy);
        }
        auto actual{ob.place_order(sellMarket)};

        OrderResult expected{
            sellMarket.get_id(),
            OrderResult::FILLED,
            std::vector<Trade>{
                Trade{ptr, sellMarket.get_id(), buy53.get_id(), 53, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellMarket.get_id(), buy52.get_id(), 52, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellMarket.get_id(), buy51.get_id(), 51, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellMarket.get_id(), buy50.get_id(), 50, 2, time_point(), Order::Side::SELL}
            },
            nullptr,
            ""
        };

        REQUIRE(actual.equals_to(expected));
    }

    SECTION("Walk the book market buy partial fill")
    {
        ob.place_order(sell50);
        ob.place_order(sell51);
        ob.place_order(sell52);
        auto actual{ob.place_order(buyMarket)};

        OrderResult expected{
            buyMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, buyMarket.get_id(), sell50.get_id(), 50, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyMarket.get_id(), sell51.get_id(), 51, 2, time_point(), Order::Side::BUY},
                Trade{ptr, buyMarket.get_id(), sell52.get_id(), 52, 2, time_point(), Order::Side::BUY}
            },
            &buyMarket,
            "Partially filled 6 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(buyMarket.volume == 2);
    }

    SECTION("Walk the book market sell partial fill")
    {
        ob.place_order(buy53);
        ob.place_order(buy52);
        ob.place_order(buy51);
        auto actual{ob.place_order(sellMarket)};

        OrderResult expected{
            sellMarket.get_id(),
            OrderResult::PARTIALLY_FILLED,
            std::vector<Trade>{
                Trade{ptr, sellMarket.get_id(), buy53.get_id(), 53, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellMarket.get_id(), buy52.get_id(), 52, 2, time_point(), Order::Side::SELL},
                Trade{ptr, sellMarket.get_id(), buy51.get_id(), 51, 2, time_point(), Order::Side::SELL}            },
            &sellMarket,
            "Partially filled 6 shares, remaining order cancelled"
        };

        REQUIRE(actual.equals_to(expected));
        REQUIRE(sellMarket.volume == 2);
    }
}

// TODO:
// add tests for what happens when someone orders, placed and then later gets filled
// add tests for ob state after each order fills
