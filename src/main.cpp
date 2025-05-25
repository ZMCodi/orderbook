#include <iostream>
#include <sstream>

#include "orderbook/OrderBook.h"

inline OrderBookState OrderBook::getState()
{
    return {
        bidMap, askMap, idMap, tradeList, orderList,
        bestBid, bestAsk, marketPrice, totalVolume
    };
}

inline Order truncPrice(const Order& order, double tickSize)
{
    return Order{order, tickSize};
}

// helper for comparing orderlists
inline bool compareOrderLists(const order_list& first, const order_list& sec, double tickSize = 0.01)
{
    if (first.size() != sec.size()) {return false;}

    auto f{first.begin()};
    auto s{sec.begin()};
    for (size_t i{}; i < first.size(); ++i)
    {
        auto expected{truncPrice(*s, tickSize)};
        if (!f->equals_to(expected)) {return false;}
        ++f;
        ++s;
    }

    return true;
}

// helpers for checking OB state after trades
inline bool checkOBState(const OrderBook& ob, const OrderBookState& state)
{
    // compare simple states
    if (ob.bestBid != state.bestBid) {return false;}
    if (ob.bestAsk != state.bestAsk) {return false;}
    if (ob.marketPrice != state.marketPrice) {return false;}
    if (ob.totalVolume != state.totalVolume) {return false;}
    std::cout << "Passed simple values\n";

    try
    {
        // compare bid map
        if (ob.bidMap.size() != state.bidMap.size()) {return false;}
        for (const auto& [price, expLevel] : state.bidMap)
        {
            const auto& [expVolume, expOrders] = expLevel;
            const auto& [actVolume, actOrders] = ob.bidMap.at(price);

            if (expVolume != actVolume) {return false;}

            if (expOrders.size() != actOrders.size()) {return false;}

            if (!compareOrderLists(expOrders, actOrders)) {return false;}
        }
        std::cout << "Pass bid map\n";

        // compare ask map
        if (ob.askMap.size() != state.askMap.size()) {return false;}
        for (const auto& [price, expLevel] : state.askMap)
        {
            const auto& [expVolume, expOrders] = expLevel;
            const auto& [actVolume, actOrders] = ob.askMap.at(price);

            if (expVolume != actVolume) {return false;}

            if (expOrders.size() != actOrders.size()) {return false;}

            if (!compareOrderLists(expOrders, actOrders)) {return false;}
        }
        std::cout << "Pass ask map\n";

        // compare id map
        if (ob.idMap.size() != state.idMap.size()) {return false;}
        for (const auto& [id, expLocation] : state.idMap)
        {
            const auto& [expPrice, expItr, expSide] = expLocation;
            const auto& [actPrice, actItr, actSide] = ob.idMap.at(id);

            if (expPrice != actPrice) {return false;}
            if (!expItr->equals_to(*actItr)) {return false;}
            if (expSide != actSide) {return false;}
        }
        std::cout << "Pass id map\n";

        // compare trade list
        if (ob.tradeList.size() != state.tradeList.size()) {return false;}
        for (size_t i{}; i < state.tradeList.size(); ++i)
        {
            if (!state.tradeList[i].equals_to(ob.tradeList[i])) {std::cout << "actual: " << ob.tradeList[i] << "\nexpected: " << state.tradeList[i]; return false;}
        }
        std::cout << "Pass trade list\n";

        // compare order list
        if (ob.orderList.size() != state.orderList.size()) {return false;}
        std::cout << "Pass order list size\n";
        for (size_t i{}; i < state.orderList.size(); ++i)
        {
            if (!state.orderList[i].equals_to(ob.orderList[i])) {std::cout << "actual: " << ob.orderList[i] << "\nexpected: " << state.orderList[i]; return false;}
        }
        std::cout << "Pass order list\n";

    } catch (const std::exception& e) 
    {
        // std::cout << "Error: " << e.what();
        return false; // .at() throws an error
    }

    return true;
}

inline void OrderBook::clear()
{
    bidMap.clear();
    askMap.clear();
    idMap.clear();
    tradeList.clear();
    orderList.clear();
    idPool.clear();
    auditList.clear();
    bestBid = -1;
    bestAsk = -1;
    marketPrice = -1;
    totalVolume = 0;
}

#include <fstream>
int main()
{
    [[maybe_unused]] OrderBook ob{};
    [[maybe_unused]] Order buy50{Order::Side::BUY, 5,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order buy50_2{Order::Side::BUY, 3,  Order::Type::LIMIT, 50};

    [[maybe_unused]] Order sell50{Order::Side::SELL, 5,  Order::Type::LIMIT, 50};
    [[maybe_unused]] Order sell50_2{Order::Side::SELL, 3,  Order::Type::LIMIT, 50};

    ob.placeOrder(buy50);
    auto id = buy50.get_id();
    ob.placeOrder(sell50_2); // this will partially fill buy50
    // auto actual{ob.cancelOrder(id)};

    // OrderResult expected{
    //     *id,
    //     OrderResult::CANCELLED,
    //     trades(),
    //     nullptr,
    //     "Order cancelled with 2 unfilled shares"
    // };

    Trade expTrade{nullptr, id, sell50_2.get_id(), 50, 3, utils::now(), Order::Side::SELL};
    OrderBookState expState{
        bid_map(), ask_map(), id_map(),
        trade_list{expTrade}, orders{buy50, sell50_2},
        -1, -1, 50, 0
    };

    std::cout << "actual: " << ob.getState();
    std::cout << "\nexpected: " << expState;
}
