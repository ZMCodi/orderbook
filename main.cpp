// for catch testing
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <chrono>
#include <map>
#include <set>

#include <uuid.h>
#include "Random.h"

// uuid generator
inline std::mt19937 engine{Random::generate()};
inline uuids::uuid_random_generator uuid_generator(engine);
class Order
{
public:
    enum Side
    {
        BUY,
        SELL
    };
    static constexpr std::array<std::string, 2> side_str{"BUY", "SELL"};

    enum Type
    {
        LIMIT,
        MARKET
    };
    static constexpr std::array<std::string, 2> type_str{"LIMIT", "MARKET"};

    Order(Side side, int volume, Type type, float price)
    : id{uuids::to_string(uuid_generator())}
    , side{side}
    , volume{volume}
    , type{type}
    , price{price}
    , timestamp{std::chrono::system_clock::now()} {}

    friend std::ostream& operator<<(std::ostream& out, const Order& order)
    {
        return out << "Order(SIDE:" << Order::side_str[order.side] << ", VOL:" << order.volume
            << ", TYPE:" << Order::type_str[order.type] << ", PRICE:" << order.price
            << " PLACED_AT:" << order.timestamp << ")";
    }

    // for testing
    bool is_equal(const Order& other) const
    {
        return id == other.id
        && side == other.side
        && volume == other.volume
        && type == other.type
        && price == other.price
        && timestamp == other.timestamp;
    }

    bool operator==(const Order& other) const
    {
        return id == other.id;
    }

    std::string_view get_id() {return id;}

private:
    std::string id;
    Side side;
    int volume;
    Type type;
    float price;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};

struct Trade
{
    std::string id{uuids::to_string(uuid_generator())};
    std::string_view buy_id;
    std::string_view sell_id;
    float price;
    int volume;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    Order::Side agressor;

    // for testing
    bool is_equal(const Trade& other) const
    {
        return id == other.id
        && buy_id == other.buy_id
        && sell_id == other.sell_id
        && price == other.price
        && volume == other.volume
        && timestamp == other.timestamp
        && agressor == other.agressor;
    }

    bool operator==(const Trade& other) const
    {
        return id == other.id;
    }
};

struct OrderResult{
    enum Status
    {
        PLACED,
        FILLED,
        PARTIALLY_FILLED,
        REJECTED,
    };

    std::string_view id;
    Status status;
    std::vector<Trade> trades;
    Order* remainingOrder;
    std::string message;

    bool operator==(const OrderResult& other) const
    {
        return id == other.id
        && status == other.status
        && trades == other.trades
        && remainingOrder == other.remainingOrder
        && message == other.message;
    }
};

class OrderBook
{
public:
    OrderBook() = default;

    OrderResult place_order(Order& order) 
    {
        return {"", OrderResult::PLACED, std::vector<Trade>(), &order, ""};
    }

    Order oldestBuyAt(float priceLevel) 
    {
        return Order{Order::BUY, 1, Order::LIMIT, priceLevel};
    }

    Order oldestSellAt(float priceLevel) 
    {
        return Order{Order::SELL, 1, Order::LIMIT, priceLevel};
    }

private:
    std::map<float, std::set<Order>> priceLevels;

};

TEST_CASE("OrderBook")
{
    OrderBook ob{};
    Order buy1{Order::BUY, 3, Order::LIMIT, 50};
    Order buy2{Order::BUY, 5, Order::LIMIT, 45};
    Order sell1{Order::SELL, 3, Order::LIMIT, 55};
    Order sell2{Order::SELL, 10, Order::LIMIT, 60};
    Order sell3{Order::SELL, 3, Order::LIMIT, 50};

    SECTION("Returns the appropriate order status when an order is placed and matched")
    {
        // unmatched order
        REQUIRE(ob.place_order(buy1) == OrderResult{buy1.get_id(), OrderResult::PLACED, std::vector<Trade>(), &buy1, ""});

        // special handling here since we cant guess the trade id's
        OrderResult result{ob.place_order(sell3)};
        // Check the status and other predictable fields
        REQUIRE(result.id == sell3.get_id());
        REQUIRE(result.status == OrderResult::FILLED);
        REQUIRE(result.remainingOrder == nullptr);

        // Check that exactly one trade was generated
        REQUIRE(result.trades.size() == 1);

        // compare the trade item
        REQUIRE(result.trades[0].buy_id == buy1.get_id());
        REQUIRE(result.trades[0].sell_id == sell3.get_id());
        REQUIRE(result.trades[0].price == Catch::Approx(100.0f));  // Use Approx for float comparison
        REQUIRE(result.trades[0].volume == 10);
        REQUIRE(result.trades[0].agressor == Order::Side::SELL);
    }

    SECTION("Takes limit orders and puts them at their price level")
    {
        ob.place_order(buy1);
        ob.place_order(buy2);
        ob.place_order(sell1);
        ob.place_order(sell2);

        REQUIRE(ob.oldestBuyAt(50.00).is_equal(buy1));
        REQUIRE(ob.oldestBuyAt(45.00).is_equal(buy1));
        REQUIRE(ob.oldestSellAt(55.00).is_equal(sell1));
        REQUIRE(ob.oldestSellAt(60.00).is_equal(sell1));
    }


}
