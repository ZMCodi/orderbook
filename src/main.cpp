#include <iostream>
#include <sstream>

#include "test_helpers.h"
#include "libraries/Timer.h"

#include <fstream>
int main(int argc, char* argv[])
{
    // Default to 1000 if no arguments provided
    int iterations = 1'000;

    // If argument provided, try to convert to int
    if (argc > 1)
    {
        try
        {
            iterations = std::stoi(argv[1]);
        }
        catch (...)
        {
            // Silently fall back to default if conversion fails
            iterations = 1'000;
        }
    }

    OrderBook ob{};
    [[maybe_unused]] Timer noPrepTimer{};
    [[maybe_unused]] long long volIn{};

    // prep the orders first
    for (int i{}; i < iterations; ++i)
    {
        // place orders around market price
        double price{};
        try
        {
            price = std::max(25.0 + Random::get(-5, 5), ob.getMarketPrice() * (1 + (Random::get(-5, 5) / 100.0)));
        } catch (std::exception&) 
        {
            // or around 50 if market price is not set
            price = std::max(0.1, 50.0 + Random::get(-5, 5));
        }

        bool isBuy{static_cast<bool>(Random::get(0, 1))};
        bool isMarket{Random::get(0, 9) < 3}; // 30% market orders
        int volume{Random::get(5, 500)};
        volIn += volume;

        if (isBuy && isMarket)
        {
            ob.placeOrder(Order::makeMarketBuy(volume));
        } else if (!isBuy && isMarket)
        {
            ob.placeOrder(Order::makeMarketSell(volume));
        } else if (isBuy && !isMarket)
        {
            ob.placeOrder(Order::makeLimitBuy(volume, price));
        } else
        {
            ob.placeOrder(Order::makeLimitSell(volume, price));
        }
    }

    double seconds{noPrepTimer.elapsed()};
    double ms{seconds * 1000};
    double us{seconds * 1'000'000};
    double ns{seconds * 1'000'000'000};

    std::ofstream out{"perf.txt"};
    std::cout.rdbuf(out.rdbuf());

    std::cout << "Time (before prep): " << seconds << " s (" 
          << ms << " ms, "
          << us << " µs, "
          << ns << " ns)\n";

    // prep and clear for actual run
    auto preppedOrders{ob.getState().orderList};
    ob.clear();
    [[maybe_unused]] Timer timer{};

    for (auto& order : preppedOrders)
    {
        ob.placeOrder(order);
    }

    seconds = timer.elapsed();
    ms = seconds * 1000;
    us = seconds * 1'000'000;
    ns = seconds * 1'000'000'000;

    std::cout << "Time (after prep): " << seconds << " s (" 
          << ms << " ms, "
          << us << " µs, "
          << ns << " ns)\n";

    auto state{ob.getState()};
    std::cout << "Orders processed: " << state.orderList.size();
    std::cout << ", Trades generated: " << state.tradeList.size();
    std::cout << ", Total volume processed: " << volIn;
    std::cout << "\nBest Bid: " << state.bestBid << ", Best Ask: " << state.bestAsk
    << ", Market Price: " << state.marketPrice << ", Total Volume: " << state.totalVolume;
    // std::cout << "\nFinal state: " << state;
}
