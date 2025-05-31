#pragma once

#include <exception>
#include <functional>

#include "orderbook/Utils.h"

template<bool ownsUUIDs>
struct TradeImpl;

using Trade = TradeImpl<false>;
using TradeCopy = TradeImpl<true>;

class OrderBook;

using callback = std::function<void(TradeCopy)>;

struct Order
{
    enum class Side
    {
        BUY,
        SELL
    };

    enum class Type
    {
        LIMIT,
        MARKET,
        STOP,
        STOP_LIMIT,
    };

    // factory functions
    static Order makeLimitBuy(int volume, double price);
    static Order makeLimitSell(int volume, double price);
    static Order makeMarketBuy(int volume);
    static Order makeMarketSell(int volume);
    static Order makeStopBuy(int volume, double stopPrice);
    static Order makeStopSell(int volume, double stopPrice);
    static Order makeStopLimitBuy(int volume, double price, double stopPrice);
    static Order makeStopLimitSell(int volume, double price, double stopPrice);

    // convenience comparisons
    bool isBuy() {return side == Side::BUY;}
    bool isSell() {return side == Side::SELL;}

    bool isLimit() {return type == Type::LIMIT;}
    bool isMarket() {return type == Type::MARKET;}
    bool isStop() {return type == Type::STOP;}
    bool isStopLimit() {return type == Type::STOP_LIMIT;}

    bool isLimitBuy() {return isLimit() && isBuy();}
    bool isLimitSell() {return isLimit() && isSell();}
    bool isMarketBuy() {return isMarket() && isBuy();}
    bool isMarketSell() {return isMarket() && isSell();}

    bool isStopBuy() {return isStop() && isBuy();}
    bool isStopSell() {return isStop() && isSell();}
    bool isStopLimitBuy() {return isStopLimit() && isBuy();}
    bool isStopLimitSell() {return isStopLimit() && isSell();}

    bool equals_to(const Order& other) const; // for testing
    callback getCallback() const {return callbackFn;} // for testing
    bool operator==(const Order& other) const;
    const uuids::uuid* get_id() const;

    const uuids::uuid* id;
    const Side side;
    int volume;
    const Type type;
    const double price;
    const double stopPrice;
    time_ timestamp;

    friend class OrderBook;
    friend Order truncPrice(const Order& order, double tickSize); // helper for testing

private:
    Order(Side side, int volume, Type type, double price, double stopPrice = -1);
    Order(const Order& order, double tickSize);
    void notify(TradeCopy trade) const;

    callback callbackFn;
};
