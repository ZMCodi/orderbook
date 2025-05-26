#pragma once

#include <chrono>
#include <list>
#include <exception>
#include <iostream>
#include "orderbook/OrderBook.h"


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

inline OrderBookState OrderBook::getState()
{
    return {
        bidMap, askMap, idMap, tradeList, orderList,
        bestBid, bestAsk, marketPrice, totalVolume
    };
}

// helpers for checking OB state after trades
inline bool checkOBState(const OrderBook& ob, const OrderBookState& state)
{
    // compare simple states
    if (ob.bestBid != state.bestBid) {return false;}
    if (ob.bestAsk != state.bestAsk) {return false;}
    if (ob.marketPrice != state.marketPrice) {return false;}
    if (ob.totalVolume != state.totalVolume) {return false;}
    // std::cout << "Passed simple values\n";

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
        // std::cout << "Pass bid map\n";

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
        // std::cout << "Pass ask map\n";

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
        // std::cout << "Pass id map\n";

        // compare trade list
        if (ob.tradeList.size() != state.tradeList.size()) {return false;}
        for (size_t i{}; i < state.tradeList.size(); ++i)
        {
            if (!state.tradeList[i].equals_to(ob.tradeList[i])) {return false;}
        }
        // std::cout << "Pass trade list\n";

        // compare order list
        if (ob.orderList.size() != state.orderList.size()) {return false;}
        for (size_t i{}; i < state.orderList.size(); ++i)
        {
            if (!state.orderList[i].equals_to(ob.orderList[i])) {return false;}
        }
        // std::cout << "Pass order list\n";

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

inline std::ostream& operator<<(std::ostream& out, const Order& o)
{
    std::stringstream str;
    str << "Order(id: " << *o.id << ", side: ";

    if (o.side == Order::Side::BUY)
    {
        str << "BUY, ";
    } else if (o.side == Order::Side::SELL)
    {
        str << "SELL, ";
    }

    str << "vol: " << o.volume << ", type: ";

    if (o.type == Order::Type::LIMIT)
    {
        str << "LIMIT, ";
    } else if (o.type == Order::Type::MARKET)
    {
        str << "MARKET, ";
    }

    str << "price: " << std::fixed << o.price << ", timestamp: "
    << o.timestamp << ")";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, const TradeCopy& t)
{
    std::stringstream str;
    str << "Trade(id: " << t.id << ", buyer_id: " << t.buyer_id
    << ", seller_id: " << t.seller_id << ", price: " << t.price
    << ", volume: " << t.volume << ", timestamp: " << t.timestamp
    << ", taker: ";

    switch (t.taker)
    {
        case Order::Side::BUY:
            str << "BUY";
            break;
        case Order::Side::SELL:
            str << "SELL";
            break;
    }

    str << ")";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, const Trade& t)
{
    return out << TradeCopy{t};
}

inline std::ostream& operator<<(std::ostream& out, const trades& t)
{
   std::stringstream str;
   str << "[\n\t";

   for (size_t i = 0; i < t.size(); ++i) {
       str << t[i];
       if (i < t.size() - 1) {
           str << ",\n\t";
       }
   }

   str << "\n]";
   out << str.str();
   return out;
}

inline std::ostream& operator<<(std::ostream& out, const OrderResult& o)
{
    std::stringstream str;
    str << "OrderResult(id: " << o.order_id << ", status: ";

    switch (o.status)
    {
    case OrderResult::PLACED:
        str << "PLACED";
        break;
    case OrderResult::FILLED:
        str << "FILLED";
        break;
    case OrderResult::PARTIALLY_FILLED:
        str << "PARTIALLY_FILLED";
        break;
    case OrderResult::REJECTED:
        str << "REJECTED";
        break;
    case OrderResult::MODIFIED:
        str << "MODIFIED";
        break;
    case OrderResult::CANCELLED:
        str << "CANCELLED";
        break;
    }

    str << ",\n\ttrades: " << o.trades << ",\nremainingOrder: "
    << o.remainingOrder << ", message: " << o.message << ")";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, order_list olist)
{
    std::stringstream str;
    str << "[";

    for (auto itr{olist.begin()}; itr != olist.end(); ++itr) {
        str << *itr;
        str << ", ";
    }

    str << "]";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, PriceLevel p)
{
    std::stringstream str;
    str << "PriceLevel(volume: " << p.volume << ", orders: " << p.orders << ")";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, ask_map a)
{
    std::stringstream str;
    str << "{\n";

    for (auto it{a.begin()}; it != a.end(); ++it)
    {
        str << '\t' << it->first << ": " << it->second << ",\n";
    }

    str << "\n}";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, bid_map a)
{
    std::stringstream str;
    str << "{\n";

    for (auto it{a.begin()}; it != a.end(); ++it)
    {
        str << '\t' << it->first << ": " << it->second << ",\n";
    }

    str << "\n}";
    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, OrderBook::Level l)
{
    std::stringstream str;
    str << "Level(price: " << l.price << ", volume: " << l.volume
    << ", orderCount: " << l.orderCount << ")";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, OrderBook::Depth d)
{
    std::stringstream str;
    str << "Depth(\n\tbids: [\n\t";

    for (auto bid : d.bids)
    {
        str << '\t' << bid << ",\n\t";
    }

    str << "],\n\tasks: [\n\t";

    for (auto ask : d.asks)
    {
        str << '\t' << ask << ",\n\t";
    }

    str << "],\n\tvolume: " << d.volume << ", bestBid: " << d.bestBid
    << ", bestAsk: " << d.bestAsk << ", marketPrice: " << d.marketPrice
    << "\n)";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, OrderLocation o)
{
    std::stringstream str;
    str << "OrderLocation(order: " << *o.itr << ", price: "
    << o.price << ", side: ";

    if (o.side == Order::Side::BUY) {str << "BUY)";}
    if (o.side == Order::Side::SELL) {str << "SELL)";}

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, id_map i)
{
     std::stringstream str;
    str << "{\n";

    for (auto it{i.begin()}; it != i.end(); ++it)
    {
        str << '\t' << *it->first << ": " << it->second << ",\n";
    }

    str << "\n}";

    return out << str.str();
}

inline std::ostream& operator<<(std::ostream& out, trade_list t)
{
    std::stringstream str;
    str << "[";

    for (size_t i = 0; i < t.size(); ++i) {
        str << t[i];
        if (i < t.size() - 1) {
            str << ", ";
        }
    }

    str << "]";
    out << str.str();
    return out;
}

inline std::ostream& operator<<(std::ostream& out, orders o)
{
    std::stringstream str;
    str << "[";

    for (size_t i = 0; i < o.size(); ++i) {
        str << o[i];
        if (i < o.size() - 1) {
            str << ", ";
        }
    }

    str << "]";
    out << str.str();
    return out;
}

inline std::ostream& operator<<(std::ostream& out, OrderBookState s)
{
    std::stringstream str;
    str << "State(\n";
    str << "\tbidMap: " << s.bidMap << ",\n";
    str << "\taskMap: " << s.askMap << ",\n";
    str << "\tidMap: " << s.idMap << ",\n";
    str << "\ttradeList: " << s.tradeList << ",\n";
    str << "\torderList: " << s.orderList << ",\n";
    str << "\tbestBid: " << s.bestBid
    << ", bestAsk: " << s.bestAsk
    << ", marketPrice: " << s.marketPrice
    << ", totalVolume: " << s.totalVolume << "\n)";

    return out << str.str();
}


