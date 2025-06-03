#include <iostream>
#include <sstream>

#include "test_helpers.h"
#include "libraries/Timer.h"

#include <fstream>

void benchmark(int argc, char* argv[])
{
    // Default to 1000 if no arguments provided
    [[maybe_unused]] int iterations = 1'000;

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
          << ns << " ns)\n\n";

    // prep and clear for actual run
    seconds = 0;
    std::cout << "sizeof(Order): " << sizeof(Order);

    for (int i{}; i < 10; ++i)
    {
        // recopy every time bcs market orders get modified in place
        auto preppedOrders{ob.getState().orderList};
        ob.clear();

        Timer timer{};

        for (auto& order : preppedOrders)
        {
            ob.placeOrder(order);
        }

        seconds += timer.elapsed();
    }

    seconds /= 10;
    ms = seconds * 1000;
    us = seconds * 1'000'000;
    ns = seconds * 1'000'000'000;

    std::cout << "\nTime (10 iterations): " << seconds << " s (" 
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

int main(int argc, char* argv[])
{
    [[maybe_unused]] auto lol = argv[argc];
    OrderBook ob{};

    [[maybe_unused]] Order buy50{Order::makeLimitBuy(2, 50)};
    [[maybe_unused]] Order buy51{Order::makeLimitBuy(2, 51)};
    [[maybe_unused]] Order buy52{Order::makeLimitBuy(2, 52)};
    [[maybe_unused]] Order buy53{Order::makeLimitBuy(2, 53)};
    [[maybe_unused]] Order buyBig53{Order::makeLimitBuy(8, 53)};
    [[maybe_unused]] Order buyMarket{Order::makeMarketBuy(8)};

    [[maybe_unused]] Order sell50{Order::makeLimitSell(2, 50)};
    [[maybe_unused]] Order sell51{Order::makeLimitSell(2, 51)};
    [[maybe_unused]] Order sell52{Order::makeLimitSell(2, 52)};
    [[maybe_unused]] Order sell53{Order::makeLimitSell(2, 53)};
    [[maybe_unused]] Order sellBig50{Order::makeLimitSell(8, 50)};

    ob.placeOrder(buy53);
    ob.placeOrder(buy52);
    ob.placeOrder(buy51);
    auto actual{ob.placeOrder(sellBig50)};
    auto id = sellBig50.get_id();

    Trade expTrade1{nullptr, buy53.get_id(), sellBig50.get_id(), 53, 2, utils::now(), Order::Side::SELL};
    Trade expTrade2{nullptr, buy52.get_id(), sellBig50.get_id(), 52, 2, utils::now(), Order::Side::SELL};
    Trade expTrade3{nullptr, buy51.get_id(), sellBig50.get_id(), 51, 2, utils::now(), Order::Side::SELL};

    sellBig50.volume = 2;

    ask_map expAM{
        {5000, PriceLevel{2, order_list{sellBig50}}}
    };

    id_map expIDM{
        {id, OrderLocation{50, expAM.at(5000).orders.begin(), OrderLocation::BID}}
    };

    sellBig50.volume = 8; // reset for orderList
    OrderBookState expState{
        bid_map(), expAM, expIDM,
        trade_list{expTrade1, expTrade2, expTrade3},
        orders{buy53, buy52, buy51, sellBig50},
        -1, 50, 51, 2
    };

    std::cout << "actual: " << ob.getState();
    std::cout << "\n\nexpected: " << expState;
}
