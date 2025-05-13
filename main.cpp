// for catch testing
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include <unordered_set>
#include <iostream>

#include "Order.h"
#include "Trade.h"
#include "OrderResult.h"
#include "OrderBook.h"

// helper for timestamps
auto time_point() {
    return std::chrono::system_clock::now();
}

// helper for comparing orderlists
bool compareOrderLists(const std::list<Order>& first, const std::list<Order>& sec)
{
    if (first.size() != sec.size()) {return false;}

    auto f{first.begin()};
    auto s{sec.begin()};
    for (size_t i{}; i < first.size(); ++i)
    {
        if (!(*f).is_equal(*s)) {return false;}
        ++f;
        ++s;
    }

    return true;
}

TEST_CASE("Order")
{
    OrderBook ob{};

    SECTION("Market order with a price throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 3, Order::Type::MARKET, 50});
        REQUIRE_THROWS(Order{Order::Side::SELL, 3, Order::Type::MARKET, 50});
    }

    SECTION("Limit order without a price throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 3, Order::Type::LIMIT});
        REQUIRE_THROWS(Order{Order::Side::SELL, 3, Order::Type::LIMIT});
    }

    SECTION("Zero or negative volume orders throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 0, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::SELL, 0, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::BUY, 0, Order::Type::MARKET});
        REQUIRE_THROWS(Order{Order::Side::SELL, 0, Order::Type::MARKET});

        REQUIRE_THROWS(Order{Order::Side::BUY, -1, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::SELL, -1, Order::Type::LIMIT, 20});
        REQUIRE_THROWS(Order{Order::Side::BUY, -1, Order::Type::MARKET});
        REQUIRE_THROWS(Order{Order::Side::SELL, -1, Order::Type::MARKET});
    }

    SECTION("Zero or negative price limit orders throws an error")
    {
        REQUIRE_THROWS(Order{Order::Side::BUY, 20, Order::Type::LIMIT, 0});
        REQUIRE_THROWS(Order{Order::Side::SELL, 20, Order::Type::LIMIT, 0});
        REQUIRE_THROWS(Order{Order::Side::BUY, 20, Order::Type::LIMIT, -1});
        REQUIRE_THROWS(Order{Order::Side::SELL, 20, Order::Type::LIMIT, -1});
    }

    SECTION("Order ID's generated are unique")
    {
        // Create persistent Order objects first
        Order order1{Order::Side::BUY, 1, Order::Type::LIMIT, 1};
        Order order2{Order::Side::BUY, 1, Order::Type::LIMIT, 1};
        Order order3{Order::Side::SELL, 1, Order::Type::MARKET};
        Order order4{Order::Side::BUY, 1, Order::Type::MARKET};
        
        // Now get the IDs from the persistent objects
        auto id1 = order1.get_id();
        auto id2 = order2.get_id();
        auto id3 = order3.get_id();
        auto id4 = order4.get_id();

        std::unordered_set<std::string_view> ids{
            id1, id2, id3, id4
        };

        REQUIRE(ids.size() == 4);
    }

    SECTION("Order handles large volume")
    {
        int max_vol{std::numeric_limits<int>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, max_vol, Order::Type::LIMIT, 1});
    }

    SECTION("Order handles large and small prices")
    {
        float max_price{std::numeric_limits<float>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, max_price});
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, 0.01f});
    }
}

TEST_CASE("OrderBook")
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5,  Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 5,  Order::Type::MARKET};
    Order buy50_2{Order::Side::BUY, 10, Order::Type::LIMIT, 50};
    Order buy50_3{Order::Side::BUY, 15, Order::Type::LIMIT, 50};

    Order sell50{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};
    Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sell55{Order::Side::SELL, 3,  Order::Type::LIMIT, 55};
    Order sell60_2{Order::Side::SELL, 30, Order::Type::LIMIT, 60};
    Order sell60_3{Order::Side::SELL, 27, Order::Type::LIMIT, 60};

    std::vector orders{
        buy50,  buy45,  buyMarket,  buy50_2,  buy50_3,
        sell50, sell60, sell55, sell60_2, sell60_3
    };

    SECTION("Takes limit orders and puts them at their price level")
    {
        ob.place_order(buy50);
        ob.place_order(buy45);
        ob.place_order(sell55);
        ob.place_order(sell60);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), std::list{buy50}));
        REQUIRE(compareOrderLists(ob.bidsAt(45.00), std::list{buy45}));
        REQUIRE(compareOrderLists(ob.asksAt(55.00), std::list{sell55}));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), std::list{sell60}));
    }

    SECTION("Handles multiple orders at the same price level")
    {
        // buys at same price
        ob.place_order(buy50);
        ob.place_order(buy50_2);
        ob.place_order(buy50_3);

        // sells at same price
        ob.place_order(sell60);
        ob.place_order(sell60_2);
        ob.place_order(sell60_3);

        REQUIRE(compareOrderLists(ob.bidsAt(50.00), std::list<Order>{
            buy50, buy50_2, buy50_3
        }));
        REQUIRE(compareOrderLists(ob.asksAt(60.00), std::list<Order>{
            sell60, sell60_2, sell60_3
        }));
    }

    SECTION("Tracks market price")
    {
        // market price not initialized
        REQUIRE_THROWS(ob.getMarketPrice());

        for (auto order : orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getMarketPrice() == Catch::Approx(50.00));
    }

    SECTION("Tracks best bid and best ask")
    {
        // best bid/ask not initialized
        REQUIRE_THROWS(ob.getBestBid());
        REQUIRE_THROWS(ob.getBestAsk());

        for (auto order: orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getBestBid() == Catch::Approx(50.00));
        REQUIRE(ob.getBestAsk() == Catch::Approx(55.00));
    }

    SECTION("Gets order by ID")
    {
        auto id1{buy50.get_id()};
        ob.place_order(buy50);
        REQUIRE(ob.getOrderByID(id1).is_equal(buy50));
    }

    SECTION("Tracks volume at price level")
    {
        // empty should be 0
        REQUIRE(ob.volumeAt(50.00) == 0);

        for (auto order: orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.volumeAt(60.00) == 67);
        REQUIRE(ob.volumeAt(55.00) == 3);
        REQUIRE(ob.volumeAt(50.00) == 25);
        REQUIRE(ob.volumeAt(45.00) == 5);
    }

    SECTION("Tracks total volume")
    {
        // empty should be 0
        REQUIRE(ob.getTotalVolume() == 0);

        for (auto order: orders)
        {
            ob.place_order(order);
        }

        REQUIRE(ob.getTotalVolume() == 100);
    }

    SECTION("Spread")
    {
        REQUIRE_THROWS(ob.getSpread());

        ob.place_order(buy50);
        REQUIRE_THROWS(ob.getSpread());

        ob.place_order(sell55);

        REQUIRE(ob.getSpread() == Catch::Approx(5.00f));
    }

    SECTION("Order cancellation/modification")
    {
        SECTION("Cancel full limit buy")
        {

        }

        SECTION("Cancel full limit sell")
        {

        }

        SECTION("Cancel partial limit buy")
        {

        }

        SECTION("Cancel partial limit sell")
        {

        }

        SECTION("Modify volume full limit buy")
        {

        }

        SECTION("Modify volume full limit sell")
        {

        }

        SECTION("Modify volume partial limit buy")
        {

        }

        SECTION("Modify volume partial limit sell")
        {

        }

        SECTION("Modify price full limit buy")
        {

        }

        SECTION("Modify price full limit sell")
        {

        }

        SECTION("Modify price partial limit buy")
        {

        }

        SECTION("Modify price partial limit sell")
        {

        }
    }

    SECTION("Depth")
    {
        // Setup some test orders at various price levels
        Order buyLow{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
        Order buyLow2{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
        Order buyLow3{Order::Side::BUY, 5, Order::Type::LIMIT, 45};
        Order buyMid{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
        Order buyMid2{Order::Side::BUY, 3, Order::Type::LIMIT, 50};
        Order buyHigh{Order::Side::BUY, 7, Order::Type::LIMIT, 55};

        Order sellLow{Order::Side::SELL, 4, Order::Type::LIMIT, 60};
        Order sellLow2{Order::Side::SELL, 4, Order::Type::LIMIT, 60};
        Order sellLow3{Order::Side::SELL, 4, Order::Type::LIMIT, 60};
        Order sellMid{Order::Side::SELL, 6, Order::Type::LIMIT, 65};
        Order sellMid2{Order::Side::SELL, 6, Order::Type::LIMIT, 65};
        Order sellHigh{Order::Side::SELL, 8, Order::Type::LIMIT, 70};

        // Place orders to populate the book
        ob.place_order(buyLow);
        ob.place_order(buyMid);
        ob.place_order(buyHigh);
        ob.place_order(sellLow);
        ob.place_order(sellMid);
        ob.place_order(sellHigh);

    // +----------+---------------+---------------+
    // | LEVEL    | BID           | ASK           |
    // |          | Volume Orders | Volume Orders |
    // +----------+---------------+---------------+
    // | 70.00    |               | 8      1      |
    // +----------+---------------+---------------+
    // | 65.00    |               | 12     2      |
    // +----------+---------------+---------------+
    // | 60.00    |               | 12     3      |
    // +----------+---------------+---------------+
    // | 55.00    | 7      1      |               |
    // +----------+---------------+---------------+
    // | 50.00    | 6      2      |               |
    // +----------+---------------+---------------+
    // | 45.00    | 15     3      |               |
    // +----------+---------------+---------------+

        SECTION("Depth centered on best bid/ask")
        {
            // Create expected depth
            OrderBook::Depth expected{
                // bids
                std::vector<OrderBook::Level>{
                    {55.0f, 7,  1},
                    {50.0f, 6,  2},
                    {45.0f, 15, 3}
                },

                // asks
                std::vector<OrderBook::Level>{
                    {60.0f, 12, 3},
                    {65.0f, 12, 2},
                    {70.0f, 8,  1}
                },

                60, // volume
                55, // best bid
                60, // best ask
                -1  // market price
            };

            // Get actual depth and compare
            auto actual = ob.getDepth(5);
            REQUIRE(actual == expected);
        }

        SECTION("Depth with limited levels")
        {
            // Create expected depth
            OrderBook::Depth expected{
                // bids
                std::vector<OrderBook::Level>{
                    {55.0f, 7, 1},
                    {50.0f, 6, 2}
                },

                // asks
                std::vector<OrderBook::Level>{
                    {60.0f, 12, 3},
                    {65.0f, 12, 2}
                },

                60, // volume
                55, // best bid
                60, // best ask
                -1  // market price
            };

            // Get actual depth with 2 levels
            auto actual = ob.getDepth(2);
            REQUIRE(actual == expected);
        }

        SECTION("Depth centered around a specific price")
        {
            // Create expected depth
            OrderBook::Depth expected{
                // bids
                std::vector<OrderBook::Level>{
                    {55.0f, 7,  1},
                    {50.0f, 6,  2},
                    {45.0f, 15, 3}
                },

                // asks
                std::vector<OrderBook::Level>{
                    {60.0f, 12, 3},
                    {65.0f, 12, 2}
                },

                60, // volume
                55, // best bid
                60, // best ask
                -1  // market price
            };

            // Get actual depth centered at 50
            auto actual = ob.getDepthAtPrice(50, 2);
            REQUIRE(actual == expected);
        }

        SECTION("Depth in a price range")
        {
            // Create expected depth
            OrderBook::Depth expected{
                // bids
                std::vector<OrderBook::Level>{
                    {55.0f, 7, 1},
                    {50.0f, 6, 2}
                },

                // asks
                std::vector<OrderBook::Level>{
                    {60.0f, 12, 3},
                },

                33, // volume
                55, // best bid
                60, // best ask
                55  // market price
            };

            // Get actual depth in range
            auto actual = ob.getDepthInRange(62.5f, 47.5f);
            REQUIRE(actual == expected);
        }

        SECTION("Empty book returns empty depth")
        {
            // Create expected empty depth
            OrderBook::Depth expected{
                std::vector<OrderBook::Level>(),
                std::vector<OrderBook::Level>(),
                0,  // volume
                -1, // best bid
                -1, // best ask
                -1  // market price
            };

            auto actual = OrderBook{}.getDepth(5);
            REQUIRE(actual == expected);
        }

        SECTION("Book with only bids returns correct depth")
        {
            OrderBook onlyBids{};
            onlyBids.place_order(buyLow);
            onlyBids.place_order(buyMid);

            // Create expected depth
            OrderBook::Depth expected{
                // bids
                std::vector<OrderBook::Level>{
                    {50.0f, 3, 1},
                    {45.0f, 5, 1}
                },

                // asks should be empty
                std::vector<OrderBook::Level>(),

                 8, // volume
                50, // best bid
                -1, // best ask
                -1  // market price
            };

            auto actual = onlyBids.getDepth(5);
            REQUIRE(actual == expected);
        }
    }
}

TEST_CASE("Order Filling")
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

    SECTION("Fill limit buy order")
    {

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

    SECTION("Partial fill limit buy")
    {

    }

    SECTION("Partial fill limit sell")
    {

    }

    SECTION("Partial fill market buy")
    {
        // execute trades until liquidity is exhausted
        // remaining order is cancelled
    }

    SECTION("Partial fill market sell")
    {
        // execute trades until liquidity is exhausted
        // remaining order is cancelled
    }

    SECTION("Walk the book limit buy")
    {

    }

    SECTION("Walk the book limit sell")
    {

    }

    SECTION("Walk the book market buy")
    {

    }

    SECTION("Walk the book market sell")
    {

    }

    SECTION("Time priority for buy")
    {

    }

    SECTION("Time priority for sell")
    {

    }

}
