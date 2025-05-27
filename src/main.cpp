#include <iostream>
#include <sstream>

#include "test_helpers.h"


int main()
{
    [[maybe_unused]] OrderBook ob{};
    [[maybe_unused]] Order buy50{Order::Side::BUY, 5,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order buy50_2{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order buy45{Order::Side::BUY, 5,  Order::Type::LIMIT, 45};
    [[maybe_unused]] Order buyMarket{Order::Side::BUY, 10,  Order::Type::MARKET};

    [[maybe_unused]] Order sell50{Order::Side::SELL, 5,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order sell50_2{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    [[maybe_unused]] Order sellMarket{Order::Side::SELL, 5,  Order::Type::MARKET};

    // no matches here
    ob.placeOrder(sell50);
    std::cout << "sell50 id: " << *sell50.id;
    auto actual{ob.placeOrder(buy50)};

    Trade expTrade{nullptr, buy50.get_id(), sell50.get_id(), 50, 5, utils::now(), Order::Side::BUY};
    OrderResult expected{
        *buy50.get_id(),
        OrderResult::FILLED,
        trades{expTrade},
        nullptr,
        "Order filled"
    };

    std::cout << "\n\nactual: " << actual;
    std::cout << "\n\nexpected: " << expected;
}
