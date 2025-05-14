#include <catch2/catch_all.hpp>
#include <unordered_set>

#include "test_helpers.h"

TEST_CASE("Order", "[order]")
{
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
        Order order1{Order::Side::BUY, 1, Order::Type::LIMIT, 1};
        Order order2{Order::Side::BUY, 1, Order::Type::LIMIT, 1};
        Order order3{Order::Side::SELL, 1, Order::Type::MARKET};
        Order order4{Order::Side::BUY, 1, Order::Type::MARKET};

        // ids are only generated when an order is placed
        OrderBook ob{};
        ob.place_order(order1);
        ob.place_order(order2);
        ob.place_order(order3);
        ob.place_order(order4);

        auto id1 = order1.get_id();
        auto id2 = order2.get_id();
        auto id3 = order3.get_id();
        auto id4 = order4.get_id();

        std::unordered_set ids{
            id1, id2, id3, id4
        };

        auto id{uuid_generator()};
        auto idstr{uuids::to_string(id)};
        uuids::uuid* idptr{&id};
        std::cout << "sizeof(uuid) " << sizeof(uuids::uuid);
        std::cout << "\nsizeof(uuid string) " << sizeof(idstr);
        std::cout << "\nsizeof(char) " << sizeof(idstr[0]);
        std::cout << "\nsizeof(uuid pointer) " << sizeof(idptr);
        std::cout << "\nsizeof(uuid sv) " << sizeof(std::string_view(idstr));
        std::cout << "\nsizeof(uint64_t) " << sizeof(uint64_t);
        std::cout << "\nuuid: " << idstr << " length: " << idstr.length();

        REQUIRE(ids.size() == 4);
    }

    SECTION("Order handles large volume")
    {
        int max_vol{std::numeric_limits<int>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, max_vol, Order::Type::LIMIT, 1});
    }

    SECTION("Test state testing")
    {
        OrderBook ob{};
        Order order1{Order::Side::BUY, 20, Order::Type::LIMIT, 50};
        Order order2{Order::Side::SELL, 20, Order::Type::LIMIT, 55};

        ob.place_order(order1);
        ob.place_order(order2);

        order_list list50{order1};
        auto itr1{list50.begin()};
        bid_map expBids{
            {50.0, PriceLevel{20, list50}}
        };

        order_list list55{order2};
        auto itr2{list55.begin()};
        ask_map expAsks{
            {55.0, PriceLevel{20, list55}}
        };


        id_map expID{
            {order1.get_id(), OrderLocation{50.0, itr1, Order::Side::BUY}},
            {order2.get_id(), OrderLocation{55.0, itr2, Order::Side::SELL}}
        };

        OrderBookState expState{
            expBids, expAsks, expID, trade_list(),
            50.0, 55.0, -1, 40
        };

        ob.setState(expState);

        REQUIRE(checkOBState(ob, expState));
    }

    SECTION("Order handles large and small prices")
    {
        float max_price{std::numeric_limits<float>::max()};
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, max_price});
        REQUIRE_NOTHROW(Order{Order::Side::BUY, 1, Order::Type::LIMIT, 0.01f});
    }
}
