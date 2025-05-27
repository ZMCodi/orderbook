#include <iostream>
#include <sstream>

#include "test_helpers.h"


int main()
{
    [[maybe_unused]] OrderBook ob{};
    [[maybe_unused]] Order buy50{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order buy45{Order::Side::BUY, 5,  Order::Type::LIMIT, 45};
    [[maybe_unused]] Order buyMarket{Order::Side::BUY, 5,  Order::Type::MARKET};
    [[maybe_unused]] Order buy50_2{Order::Side::BUY, 10, Order::Type::LIMIT, 50};
    [[maybe_unused]] Order buy50_3{Order::Side::BUY, 15, Order::Type::LIMIT, 50};

    [[maybe_unused]] Order sell50{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order sell60{Order::Side::SELL, 10, Order::Type::LIMIT, 60};
    [[maybe_unused]] Order sell55{Order::Side::SELL, 3,  Order::Type::LIMIT, 55};
    [[maybe_unused]] Order sell60_2{Order::Side::SELL, 30, Order::Type::LIMIT, 60};
    [[maybe_unused]] Order sell60_3{Order::Side::SELL, 27, Order::Type::LIMIT, 60};
    [[maybe_unused]] Order sellMarket{Order::Side::SELL, 5,  Order::Type::MARKET};

    // no matches here
    std::vector<std::reference_wrapper<Order>> orders{
        buy50,  buy45,  buyMarket,  buy50_2,  buy50_3,
        sell50, sell60, sell55, sell60_2, sell60_3
    };

    for (auto order: orders)
    {
        ob.placeOrder(order);
    }

    std::cout << "state: " << ob.getState();
    std::cout << ob.getState().askMap.at(6000);
    std::cout << "\n\nvolume: " << ob.volumeAt(60.00);
}
