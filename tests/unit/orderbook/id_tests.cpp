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
            ob.placeOrder(order);
            ids.insert(*order.get_id());

            // add copy
            Order ordercpy{order};
            ob.placeOrder(ordercpy);
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
        ob.placeOrder(buy50);
        auto* id1{ob.placeOrder(sell50).trades[0].id};

        ob.placeOrder(sell60);
        auto* id2{ob.placeOrder(buyMarket).trades[0].id};

        ob.placeOrder(sellMarket);
        auto* id3{ob.placeOrder(buy45).trades[0].id};

        REQUIRE(std::unordered_set{id1, id2, id3}.size() == 3);

        // check trade ID and order ID are different
        REQUIRE(std::unordered_set{
            *id1, *id2, *id3,
            *buy50.get_id(), *sell50.get_id(),
            *sell60.get_id(), *buyMarket.get_id(),
            *sellMarket.get_id(), *buy45.get_id()
        }.size() == 8);


        id_pool ids_pool{ob.getIDPool()};
        REQUIRE(ids_pool.contains(*id1));
        REQUIRE(ids_pool.contains(*id2));
        REQUIRE(ids_pool.contains(*id3));
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
        REQUIRE(*buy50.get_id() == uuids::uuid{});
        REQUIRE(ob.getIDPool().empty());
    }

    SECTION("IDs persist in the orderbook after order is filled or rejected")
    {
        // these orders will be filled
        ob.placeOrder(buy50);
        ob.placeOrder(sell50);

        // this order will be rejected
        ob.placeOrder(buyMarket);

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
        ob.placeOrder(buy50);
        ob.placeOrder(sell60);

        id_map idMap{ob.getState().idMap};
        REQUIRE(idMap.contains(buy50.get_id()));
        REQUIRE(idMap.contains(sell60.get_id()));

        // buy50 will be filled
        ob.placeOrder(sell50);
        REQUIRE(!idMap.contains(buy50.get_id()));

        // sellMarket will be rejected
        ob.placeOrder(sellMarket);
        REQUIRE(!idMap.contains(sellMarket.get_id()));
    }

    SECTION("Re-placing the same order")
    {
        ob.placeOrder(buy45);
        auto actual{ob.placeOrder(buy45)};
        auto old_id{actual.order_id};

        OrderResult expected{
            *buy45.get_id(),
            OrderResult::REJECTED,
            trades(),
            &ob.getOrderByID(buy45.get_id()),
            "Order already exists"
        };
        REQUIRE(actual.equals_to(expected));

        // to re-place an identical order, set id to nullptr
        buy45.id = nullptr;
        auto actual2{ob.placeOrder(buy45)};
        REQUIRE(old_id != actual.order_id); // new ID is generated

        OrderResult expected2{
            *buy45.get_id(),
            OrderResult::PLACED,
            trades(),
            &ob.getOrderByID(buy45.get_id()),
            "Order placed"
        };
        REQUIRE(actual2.equals_to(expected2));

        // order with garbage ID is also rejected
        auto fakeID{utils::uuid_generator()};
        sell50.id = &fakeID;
        auto actual3{ob.placeOrder(sell50)};

        OrderResult expected3{
            fakeID, // no new ID is generated
            OrderResult::REJECTED,
            trades(),
            &sell50,
            "Non-null ID"
        };
        REQUIRE(actual3.equals_to(expected3));
    }
}
