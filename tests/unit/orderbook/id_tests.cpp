#include <catch2/catch_all.hpp>

#include "test_helpers.h"

TEST_CASE("ID generation", "[orderbook][id]")
{
    OrderBook ob{};

    Order buy50{Order::makeLimitBuy(3, 50)};
    Order buy45{Order::makeLimitBuy(5, 45)};
    Order buyMarket{Order::makeMarketBuy(10)};

    Order sell50{Order::makeLimitSell(3, 50)};
    Order sell60{Order::makeLimitSell(10, 60)};
    Order sellMarket{Order::makeMarketSell(5)};

    SECTION("ID is generated strictly after placing an order")
    {
        REQUIRE(!buy50.get_id());
        ob.placeOrder(buy50);
        REQUIRE(*buy50.get_id() != uuids::uuid{});

        REQUIRE(!sell50.get_id());
        ob.placeOrder(sell50);
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
            Order ordercpy{order};

            ob.placeOrder(order);
            ids.insert(*order.get_id());

            // add copy
            ob.placeOrder(ordercpy);
            ids.insert(*ordercpy.get_id());
        }

        REQUIRE(ids.size() == 12);

        id_pool ids_pool{ob.getIDPool()};
        for (auto id : ids)
        {
            REQUIRE(ids_pool.contains(id));
        }
    }

    SECTION("Generate unique Trade IDs and store them")
    {
        ob.placeOrder(buy50);
        auto id1{ob.placeOrder(sell50).trades.at(0).id};

        ob.placeOrder(sell60);
        auto id2{ob.placeOrder(buyMarket).trades.at(0).id};

        ob.placeOrder(buy45);
        auto id3{ob.placeOrder(sellMarket).trades.at(0).id};

        REQUIRE(std::unordered_set{id1, id2, id3}.size() == 3);

        // check trade ID and order ID are different
        REQUIRE(std::unordered_set{
            id1, id2, id3,
            *buy50.get_id(), *sell50.get_id(),
            *sell60.get_id(), *buyMarket.get_id(),
            *sellMarket.get_id(), *buy45.get_id()
        }.size() == 9);


        id_pool ids_pool{ob.getIDPool()};
        REQUIRE(ids_pool.contains(id1));
        REQUIRE(ids_pool.contains(id2));
        REQUIRE(ids_pool.contains(id3));
    }

    SECTION("Rejected orders have ID")
    {
        // this would be rejected since there is no liquidity
        ob.placeOrder(buyMarket);
        REQUIRE(*buyMarket.get_id() != uuids::uuid{});
    }

    SECTION("Invalid orders don't have ID")
    {
        // do this manually since order constructor will throw
        buy50.volume = -1;
        try {ob.placeOrder(buy50);}
        catch (std::exception&) {}

        // id is uninitialized
        REQUIRE(!buy50.get_id());
        REQUIRE(ob.getIDPool().empty());
    }

    SECTION("IDs persist in the orderbook after order is filled or rejected")
    {
        // these orders will be filled
        ob.placeOrder(buy50);
        ob.placeOrder(sell50);

        // this order will be rejected
        ob.placeOrder(buyMarket);

        auto id1{*buy50.get_id()};
        auto id2{*sell50.get_id()};
        auto id3{*buyMarket.get_id()};

        id_pool ids_pool{ob.getIDPool()};
        REQUIRE(ids_pool.contains(id1));
        REQUIRE(ids_pool.contains(id2));
        REQUIRE(ids_pool.contains(id3));

    }

    SECTION("Live orders are stored in the ID Map and removed if filled or rejected")
    {
        // these orders are not filled
        ob.placeOrder(buy50);
        ob.placeOrder(sell60);

        id_map idMap{ob.getState().idMap};
        REQUIRE(idMap.contains(buy50.get_id()));
        REQUIRE(idMap.contains(sell60.get_id()));

        // buy50 will be filled
        ob.placeOrder(sell50);
        idMap = ob.getState().idMap;
        REQUIRE(!idMap.contains(buy50.get_id()));

        // sellMarket will be rejected
        ob.placeOrder(sellMarket);
        idMap = ob.getState().idMap;
        REQUIRE(!idMap.contains(sellMarket.get_id()));
    }

    SECTION("Re-placing the same order")
    {
        auto first{ob.placeOrder(buy45)};
        auto old_ts{buy45.timestamp};
        auto old_id{*buy45.id};

        // place the same order again
        auto actual{ob.placeOrder(buy45)};
        auto new_ts{buy45.timestamp};
        auto new_id{*buy45.id};

        // Order should be restamped
        REQUIRE(old_ts != new_ts);
        REQUIRE(old_id != new_id);

        OrderResult expected{
            *buy45.get_id(),
            OrderResult::PLACED,
            trades{},
            &ob.getOrderByID(buy45.get_id()),
            "Order placed"
        };
        REQUIRE(actual.equals_to(expected));

        // order with garbage ID is also restamped
        auto fakeID{utils::uuid_generator()};
        sell50.id = &fakeID;

        auto actual2{ob.placeOrder(sell50)};
        new_id = *sell50.id;

        REQUIRE(fakeID != new_id);

        OrderResult expected2{
            *sell50.get_id(), // new ID is generated
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(sell50.get_id()),
            "Order placed"
        };
        REQUIRE(actual2.equals_to(expected2));

        // fake ID is not stored in order book
        REQUIRE(!ob.getIDPool().contains(fakeID));
    }
}
