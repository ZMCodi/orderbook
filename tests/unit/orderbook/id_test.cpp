#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("ID generation", "[orderbook][id]")
{
    OrderBook ob{};

    Order buy50{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};
    Order buy45{Order::Side::BUY, 5,  Order::Type::LIMIT, 45};
    Order buyMarket{Order::Side::BUY, 10,  Order::Type::MARKET};

    Order sell50{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};
    Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    Order sellMarket{Order::Side::SELL, 5,  Order::Type::MARKET};

    SECTION("ID is generated strictly after placing an order")
    {
        REQUIRE(*buy50.get_id() == uuids::uuid{});
        ob.place_order(buy50);
        REQUIRE(*buy50.get_id() != uuids::uuid{});

        REQUIRE(*sell50.get_id() == uuids::uuid{});
        ob.place_order(sell50);
        REQUIRE(*sell50.get_id() != uuids::uuid{});
    }

    SECTION("Generate unique order IDs and stores them")
    {
        // ids are only generated when an order is placed
        std::vector orders{
            buy50,  buy45,  buyMarket,
            sell50, sell60, sellMarket
        };

        std::unordered_set<uuids::uuid> ids{};

        for (auto order : orders)
        {
            ob.place_order(order);
            ids.insert(*order.get_id());

            // add copy
            Order ordercpy{order};
            ob.place_order(ordercpy);
            ids.insert(*ordercpy.get_id());
        }

        REQUIRE(ids.size() == 8);

        id_pool ids_pool{ob.getIDPool()};
        for (auto id : ids)
        {
            REQUIRE(ids_pool.contains(id));
        }
    }

    SECTION("Generate unique Trade IDs and store them")
    {
        ob.place_order(buy50);
        auto *id1{ob.place_order(sell50).trades[0]->id};

        ob.place_order(sell60);
        auto *id2{ob.place_order(buyMarket).trades[0]->id};

        ob.place_order(sellMarket);
        auto *id3{ob.place_order(buy45).trades[0]->id};

        REQUIRE(std::unordered_set{id1, id2, id3}.size() == 3);

        id_pool ids_pool{ob.getIDPool()};
        REQUIRE(ids_pool.contains(*id1));
        REQUIRE(ids_pool.contains(*id2));
        REQUIRE(ids_pool.contains(*id3));
    }

    SECTION("Rejected orders have ID")
    {
        // this would be rejected since there is no liquidity
        ob.place_order(buyMarket);
        REQUIRE(*buyMarket.get_id() != uuids::uuid{});
    }

    SECTION("IDs persist in the orderbook after order is filled or rejected")
    {
        // these orders will be filled
        ob.place_order(buy50);
        ob.place_order(sell50);

        // this order will be rejected
        ob.place_order(buyMarket);

        auto *id1{buy50.get_id()};
        auto *id2{sell50.get_id()};
        auto *id3{buyMarket.get_id()};

        id_pool ids_pool{ob.getIDPool()};
        REQUIRE(ids_pool.contains(*id1));
        REQUIRE(ids_pool.contains(*id2));
        REQUIRE(ids_pool.contains(*id3));

    }

    SECTION("Live orders are stored in the ID Map and removed if filled or rejected")
    {
        // these orders are not filled
        ob.place_order(buy50);
        ob.place_order(sell60);

        id_map idMap{ob.getState().idMap};
        REQUIRE(idMap.contains(buy50.get_id()));
        REQUIRE(idMap.contains(sell60.get_id()));

        // buy50 will be filled
        ob.place_order(sell50);
        REQUIRE(!idMap.contains(buy50.get_id()));

        // sellMarket will be rejected
        ob.place_order(sellMarket);
        REQUIRE(!idMap.contains(sellMarket.get_id()));
    }
}
