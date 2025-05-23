#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("Depth", "[orderbook][depth]")
{
    OrderBook ob{};

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
    ob.placeOrder(buyLow);
    ob.placeOrder(buyMid);
    ob.placeOrder(buyHigh);
    ob.placeOrder(sellLow);
    ob.placeOrder(sellMid);
    ob.placeOrder(sellHigh);

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
                {55.0, 7,  1},
                {50.0, 6,  2},
                {45.0, 15, 3}
            },

            // asks
            std::vector<OrderBook::Level>{
                {60.0, 12, 3},
                {65.0, 12, 2},
                {70.0, 8,  1}
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
                {55.0, 7, 1},
                {50.0, 6, 2}
            },

            // asks
            std::vector<OrderBook::Level>{
                {60.0, 12, 3},
                {65.0, 12, 2}
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
                {55.0, 7,  1},
                {50.0, 6,  2},
                {45.0, 15, 3}
            },

            // asks
            std::vector<OrderBook::Level>{
                {60.0, 12, 3},
                {65.0, 12, 2}
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
                {55.0, 7, 1},
                {50.0, 6, 2}
            },

            // asks
            std::vector<OrderBook::Level>{
                {60.0, 12, 3},
            },
            33, // volume
            55, // best bid
            60, // best ask
            55  // market price
        };

        // Get actual depth in range
        auto actual = ob.getDepthInRange(62.5, 47.5);
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
        onlyBids.placeOrder(buyLow);
        onlyBids.placeOrder(buyMid);

        // Create expected depth
        OrderBook::Depth expected{
            // bids
            std::vector<OrderBook::Level>{
                {50.0, 3, 1},
                {45.0, 5, 1}
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

    SECTION("Book with only asks returns correct depth")
    {
        OrderBook onlyAsks{};
        onlyAsks.placeOrder(sellLow);
        onlyAsks.placeOrder(sellMid);

        // Create expected depth
        OrderBook::Depth expected{
            // bids should be empty
            std::vector<OrderBook::Level>(),

            // asks
            std::vector<OrderBook::Level>{
                {60.0, 4, 1},
                {65.0, 6, 1}
            },

            10, // volume
            -1, // best bid
            60, // best ask
            -1  // market price
        };

        auto actual = onlyAsks.getDepth(5);
        REQUIRE(actual == expected);
    }

    SECTION("Depth with no levels returns empty depth")
    {
        // empty depth still captures the state of the orderbook
        OrderBook::Depth expected{
            std::vector<OrderBook::Level>(),
            std::vector<OrderBook::Level>(),
            60,  // volume
            55, // best bid
            60, // best ask
            -1  // market price
        };

        auto actual = ob.getDepth(0);
        REQUIRE(actual == expected);
    }

    SECTION("Stress testing the depth")
    {
        // Create large number of orders on both sides
        // and fill in the expected levels
        std::vector<OrderBook::Level> expectedBids;
        std::vector<OrderBook::Level> expectedAsks;

        // the buys would be at 59.95, 59.94, 59.93, ..., 49.96
        // the sells would be at 60.05, 60.06, 60.07, ..., 70.04
        // each level would have 1 order with 1 volume
        for (int i = 0; i < 1000; ++i)
        {
            // make the price differences granular
            ob.placeOrder(Order::makeLimitBuy(1, 59.95 - i * 0.01));
            ob.placeOrder(Order::makeLimitSell(1, 60.05 + i * 0.01));

            expectedBids.push_back({59.95 - i * 0.01, 1, 1});
            expectedAsks.push_back({60.05 + i * 0.01, 1, 1});
        }

        OrderBook::Depth expected{
            expectedBids,
            expectedAsks,
            2000, // volume
            59.95, // best bid
            60.05, // best ask
            -1      // market price
        };
        // Get actual depth
        auto actual = ob.getDepth(1000);
        REQUIRE(actual == expected);
    }
}
