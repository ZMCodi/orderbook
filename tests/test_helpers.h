#pragma once

#include <chrono>
#include <list>
#include <exception>
#include <iostream>
#include "orderbook/OrderBook.h"

// helper for timestamps
inline auto time_point() {
    return std::chrono::system_clock::now();
}

// helper for comparing orderlists
inline bool compareOrderLists(const std::list<Order>& first, const std::list<Order>& sec)
{
    if (first.size() != sec.size()) {return false;}

    auto f{first.begin()};
    auto s{sec.begin()};
    for (size_t i{}; i < first.size(); ++i)
    {
        if (!(*f).equals_to(*s)) {return false;}
        ++f;
        ++s;
    }

    return true;
}

inline bool checkOBState(const OrderBook& ob, const OrderBookState& state)
{
    // compare simple states
    if (ob.bestBid != state.bestBid) {return false;}
    if (ob.bestAsk != state.bestAsk) {return false;}
    if (ob.marketPrice != state.marketPrice) {return false;}
    if (ob.totalVolume != state.totalVolume) {return false;}

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

    } catch (const std::exception& e) 
    {
        // put this here first to test the method actually works
        std::cout << "Error: " << e.what();
        return false; // .at() throws an error
    }

    return true;
}
