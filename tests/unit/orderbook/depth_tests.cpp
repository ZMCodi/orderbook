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
        onlyBids.placeOrder(buyLow);
        onlyBids.placeOrder(buyMid);

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
