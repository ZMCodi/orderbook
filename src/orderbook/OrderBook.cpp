#include "orderbook/OrderBook.h"
#include <iostream>

const order_list OrderBook::emptyOrders{};

bool OrderAudit::equals_to(const OrderAudit& other) const
{
    return id == other.id
    && volume_delta == other.volume_delta;
}

bool OrderBook::Level::operator==(const Level& other) const
{
    return std::abs(price - other.price) < 0.0001
    && volume == other.volume
    && orderCount == other.orderCount;
}

bool OrderBook::Depth::operator==(const Depth& other) const
{
    return bids == other.bids
    && asks == other.asks
    && volume == other.volume
    && std::abs(bestBid - other.bestBid) < 0.0001
    && std::abs(bestAsk - other.bestAsk) < 0.0001
    && std::abs(marketPrice - other.marketPrice) < 0.0001;
}

void OrderBook::stampOrder(Order& order)
{
    // generate a unique id and timestamp for the order
    auto id{utils::uuid_generator()};
    time_ ts{utils::now()};

    // store in id pool for persistent storage and get pointer
    auto [it, _] = idPool.insert(id);

    // stamp the original order object first
    order.id = &(*it);
    order.timestamp = ts;
    orderList.push_back(order); // bookkeeping
}

OrderResult OrderBook::placeOrder(Order& order, callback callbackFn)
{
    if (order.volume <= 0) 
    {
        throw std::invalid_argument{"Volume has to be positive"};
    }

    stampOrder(order);

    // for market orders, no need for a copy
    if (order.isMarket()) 
    {
        order.callbackFn = callbackFn;
        return matchOrder(order);
    }

    // local active copy that is actually processed with truncated price
    Order activeCopy{order, tickSize};
    activeCopy.callbackFn = callbackFn;

    // for storing
    tick_t tickPrice{utils::convertTick(activeCopy.price, tickSize)};

    // match with existing orders and return result
    auto result{matchOrder(activeCopy)};

    // if order remains unfilled
    if (activeCopy.volume > 0)
    {
        // store in bid/ask map and update any relevant fields
        dispatchBySide(activeCopy.side, [&](auto& orderMap){
            storeActiveOrder(activeCopy, orderMap, tickPrice, result);
        });
    }

    return result;
}

OrderResult OrderBook::placeOrder(Order&& order, callback callbackFn)
{
    return placeOrder(order, callbackFn);
}

// matching logic dispatcher
OrderResult OrderBook::matchOrder(Order& order)
{
    switch (order.type) {
        case Order::Type::LIMIT:
            return order.side == Order::Side::BUY 
                ? matchOrderTemplate<Order::Type::LIMIT>(order, askMap)
                : matchOrderTemplate<Order::Type::LIMIT>(order, bidMap);
        case Order::Type::MARKET:
            return order.side == Order::Side::BUY
                ? matchOrderTemplate<Order::Type::MARKET>(order, askMap)
                : matchOrderTemplate<Order::Type::MARKET>(order, bidMap);
        default:
            return {*order.id, OrderResult::REJECTED, trades{}, nullptr, "Something went wrong"};
    }
}

void OrderBook::genTrade(const Order& buyer, const Order& seller, double price,
                        int volume, Order::Side side, trades& generatedTrades)
{
    auto tradeId{utils::uuid_generator()};
    auto [it, _] = idPool.insert(tradeId);
    Trade trade{&(*it), buyer.id, seller.id, price, volume, utils::now(), side};
    generatedTrades.push_back(trade);

    // trigger callbacks
    buyer.notify(trade);
    seller.notify(trade);

    // internal bookkeeping
    tradeList.push_back(trade);
    marketPrice = price;
}

const order_list& OrderBook::ordersAt(double priceLevel)
{
    tick_t tickPrice{utils::convertTick(priceLevel, tickSize)};

    // determine to check at bidMap or askMap
    if (bestBid != -1 && priceLevel <= bestBid) // bidMap
    {
        return priceLevelQuery<false>(bidMap, tickPrice);
    } else if (bestAsk != -1 && priceLevel >= bestAsk) // askMap
    {
        return priceLevelQuery<false>(askMap, tickPrice);
    }

    return OrderBook::emptyOrders;
}

const order_list& OrderBook::bidsAt(double priceLevel)
{
    return ordersAt(priceLevel);
}

const order_list& OrderBook::asksAt(double priceLevel)
{
    return ordersAt(priceLevel);
}

int OrderBook::volumeAt(double priceLevel)
{
    tick_t tickPrice{utils::convertTick(priceLevel, tickSize)};

    // determine to check at bidMap or askMap
    if (bestBid != -1 && priceLevel <= bestBid)
    {
        return priceLevelQuery<true>(bidMap, tickPrice);
    } else if (bestAsk != -1 && priceLevel >= bestAsk)
    {
        return priceLevelQuery<true>(askMap, tickPrice);
    }

    return 0;
}

// center around best bid/ask
OrderBook::Depth OrderBook::getDepth(size_t levels)
{
    std::vector<OrderBook::Level> bids{};
    std::vector<OrderBook::Level> asks{};

    auto bidIt{bidMap.begin()};
    auto askIt{askMap.begin()};
    size_t i{};

    while (true)
    {
        if (i >= levels) {break;} // got `level` levels
        if (bidIt == bidMap.end() && askIt == askMap.end()) {break;} // both iterators are at end

        if (bidIt != bidMap.end()) // add and increment as long as we haven't reached the end
        {
            bids.emplace_back(
                bidIt->first * tickSize, bidIt->second.volume,
                bidIt->second.orders.size()
            );
            ++bidIt;
        }

        if (askIt != askMap.end())
        {
            asks.emplace_back(
                askIt->first * tickSize, askIt->second.volume,
                askIt->second.orders.size()
            );
            ++askIt;
        }

        ++i;
    }

    return {bids, asks, totalVolume, bestBid, bestAsk, marketPrice};
}

// center around a given price
OrderBook::Depth OrderBook::getDepthAtPrice(double price, size_t levels)
{
    std::vector<OrderBook::Level> bids{};
    std::vector<OrderBook::Level> asks{};

    if (price < bestBid) // centered around a bid price
    {

        bids = getLevelsAtPrice(bidMap, levels, price);

        // for asks just go `levels` level from best ask
        asks = getLevels(askMap, levels);

    } else if (price > bestAsk) // centered around an ask price
    {
        // logic here is basically the same
        asks = getLevelsAtPrice(askMap, levels, price);
        bids = getLevels(bidMap, levels);

    } else // centered around best bid/ask or gap in between
    {
        return getDepth(levels);
    }

    return {bids, asks, totalVolume, bestBid, bestAsk, marketPrice};
}

// depth in a given range
OrderBook::Depth OrderBook::getDepthInRange(double minPrice, double maxPrice)
{
    std::vector<OrderBook::Level> bids{};
    std::vector<OrderBook::Level> asks{};

    if (maxPrice >= bestAsk && minPrice >= bestAsk) // whole range is just in asks
    {
        asks = getLevelsOneSided(askMap, minPrice, maxPrice);

    } else if (minPrice <= bestBid && maxPrice <= bestBid) // whole range just in bids
    {
        bids = getLevelsOneSided(bidMap, minPrice, maxPrice);

    } else // go from best ask to max and best bid to min
    {
        for (auto it{askMap.begin()}; it != askMap.end(); ++it)
        {
            if (it->first * tickSize > maxPrice) {break;}
            asks.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
        }

        for (auto it{bidMap.begin()}; it != bidMap.end(); ++it)
        {
            if (it->first * tickSize < minPrice) {break;}
            bids.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
        }
    }

    return {bids, asks, totalVolume, bestBid, bestAsk, marketPrice};
}
