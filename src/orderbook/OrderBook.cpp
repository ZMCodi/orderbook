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

OrderResult OrderBook::placeOrder(Order& order)
{
    if (order.volume <= 0) 
    {
        throw std::invalid_argument{"Volume has to be positive"};
    }
    // generate a unique id and timestamp for the order
    auto id{utils::uuid_generator()};
    time_ ts{utils::now()};

    // store in id pool for persistent storage and get pointer
    auto [it, _] = idPool.insert(id);

    // stamp the original order object first
    // OPTIMIZE: can skip stamping for rvalues since user wont keep
    order.id = &(*it);
    order.timestamp = ts;

    // bookkeeping
    orderList.push_back(order);

    // for market orders, no need for a copy
    if (order.type == Order::Type::MARKET) {return matchOrder(order);}

    // local active copy that is actually processed with truncated price
    Order activeCopy{order, tickSize};
    double truncPrice{activeCopy.price};
    tick_t tickPrice{utils::convertTick(truncPrice, tickSize)};

    // match with existing orders and return result
    auto result{matchOrder(activeCopy)};

    // if order remains unfilled
    if (activeCopy.volume > 0)
    {
        // update relevant fields
        totalVolume += activeCopy.volume;

        // put in bid/ask map
        order_list::iterator activeItr;

        // some code duplication here since bid_map and ask_map are diff types
        if (activeCopy.side == Order::Side::BUY)
        {
            // update best bid if better
            if (activeCopy.price > bestBid) {bestBid = activeCopy.price;}

            // add the order to the end of the orderlist
            // and update volume at PriceLevel
            auto& pLevel{bidMap[tickPrice]};
            pLevel.volume += activeCopy.volume;
            pLevel.orders.push_back(std::move(activeCopy));
            activeItr = std::prev(pLevel.orders.end());
        } else
        {
            // update best ask if better
            if (bestAsk == -1 || activeCopy.price < bestAsk) {bestAsk = activeCopy.price;}

            auto& pLevel{askMap[tickPrice]};
            pLevel.volume += activeCopy.volume;
            pLevel.orders.push_back(std::move(activeCopy));
            activeItr = std::prev(pLevel.orders.end());
        }

        // add to idMap
        idMap[order.id] = OrderLocation{truncPrice, activeItr, order.side};
        result.remainingOrder = &(*activeItr); // result points to remaining order

    }

    return result;
}

OrderResult OrderBook::placeOrder(Order&& order)
{
    return placeOrder(order);
}

OrderResult OrderBook::matchOrder(Order& order)
{
    return {*order.id, OrderResult::PLACED, trades{}, nullptr, "Order placed"};
}

OrderResult OrderBook::placeOrder(Order& order, callback callbackFn)
{
    [[maybe_unused]] auto lol = callbackFn;
    return {*order.get_id(), OrderResult::FILLED, trades(), &order, ""};
}

OrderResult OrderBook::placeOrder(Order&& order, callback callbackFn)
{
    return placeOrder(order, callbackFn);
}

OrderResult OrderBook::cancelOrder(const uuids::uuid* id)
{
    return {*id, OrderResult::FILLED, trades(), nullptr, ""};
}

OrderResult OrderBook::modifyVolume(const uuids::uuid* id, int volume)
{
    [[maybe_unused]] auto lol = volume * 2;
    return {*id, OrderResult::FILLED, trades(), nullptr, ""};
}

OrderResult OrderBook::modifyPrice(const uuids::uuid* id, double price)
{
    [[maybe_unused]] auto lol = price * 2;
    return {*id, OrderResult::FILLED, trades(), nullptr, ""};
}

OrderResult OrderBook::cancelOrder(const uuids::uuid& id)
{
    return {id, OrderResult::FILLED, trades(), nullptr, ""};
}

OrderResult OrderBook::modifyVolume(const uuids::uuid& id, int volume)
{
    [[maybe_unused]] auto lol = volume * 2;
    return {id, OrderResult::FILLED, trades(), nullptr, ""};
}

OrderResult OrderBook::modifyPrice(const uuids::uuid& id, double price)
{
    [[maybe_unused]] auto lol = price * 2;
    return {id, OrderResult::FILLED, trades(), nullptr, ""};
}

bool OrderBook::registerCallback(const uuids::uuid* id, callback callbackFn)
{
    [[maybe_unused]] auto lol = id;
    [[maybe_unused]] auto lol2 = callbackFn;
    return false;
}

bool OrderBook::removeCallback(const uuids::uuid* id)
{
    [[maybe_unused]] auto lol = id;
    return false;
}

bool OrderBook::registerCallback(const uuids::uuid& id, callback callbackFn)
{
    [[maybe_unused]] auto lol = id;
    [[maybe_unused]] auto lol2 = callbackFn;
    return false;
}

bool OrderBook::removeCallback(const uuids::uuid& id)
{
    [[maybe_unused]] auto lol = id;
    return false;
}

const order_list& OrderBook::bidsAt(double priceLevel)
{
    tick_t tickPrice{utils::convertTick(priceLevel, tickSize)};
    auto it{bidMap.find(tickPrice)};

    // if no bids, return empty list
    if (it == bidMap.end())
    {
        return OrderBook::emptyOrders;
    }

    return it->second.orders;
}

const order_list& OrderBook::asksAt(double priceLevel)
{
    tick_t tickPrice{utils::convertTick(priceLevel, tickSize)};
    auto it{askMap.find(tickPrice)};

    // if no asks, return empty list
    if (it == askMap.end())
    {
        return OrderBook::emptyOrders;
    }

    return it->second.orders;
}

const Order& OrderBook::getOrderByID(const uuids::uuid* id)
{
    try
    {
        return *idMap.at(id).itr;
    } catch (const std::out_of_range&)
    {
        throw std::invalid_argument{"Order is no longer active"};
    }
}

const Order& OrderBook::getOrderByID(const uuids::uuid& id)
{
    // look in the idPool first if the id exists
    auto it{idPool.find(id)};

    if (it == idPool.end())
    {
        throw std::invalid_argument{"ID does not exist"};
    }

    return getOrderByID(&(*it));
}

int OrderBook::volumeAt(double priceLevel)
{
    tick_t tickPrice{utils::convertTick(priceLevel, tickSize)};
    // determine to check at bidMap or askMap
    if (priceLevel <= bestBid)
    {
        auto it{bidMap.find(tickPrice)};
        if (it != bidMap.end())
        {
            return it->second.volume;
        }
    } else if (priceLevel >= bestAsk)
    {
        // std::cout << askMap[priceLevel];
        auto it{askMap.find(tickPrice)};
        if (it != askMap.end())
        {
            std::cout << "here";
            return it->second.volume;
        }
    }

    return 0;
}

// OPTIMIZE: this could probably be just one loop
// center around best bid/ask
OrderBook::Depth OrderBook::getDepth(size_t levels)
{
    // get bids first
    std::vector<OrderBook::Level> bids{};
    size_t i{};
    for (auto it{bidMap.begin()}; it != bidMap.end(); ++it)
    {
        if (i >= levels) {break;}
        bids.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
        ++i;
    }

    // get asks
    std::vector<OrderBook::Level> asks{};
    i = 0;
    for (auto it{askMap.begin()}; it != askMap.end(); ++it)
    {
        if (i >= levels) {break;}
        asks.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
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
        // start at best bid and go until `levels` levels below the price
        // iterate and find the level of the price after it (in case the price itself doesnt exist)
        auto endIt{bidMap.begin()};
        while (endIt != bidMap.end())
        {
            if (endIt->first * tickSize < price) // found the first level after
            {
                // move it the remaining levels
                // it should end one step over for a half open range
                if (std::distance(endIt, bidMap.end()) >= static_cast<int>(levels))
                {
                    // make sure we dont advance past the end iterator here
                    std::advance(endIt, levels);
                } else
                {
                    endIt = bidMap.end();
                }
                break;
            }
            ++endIt;
        }

        // bids should be everything from best bid to the end iterator
        for (auto it{bidMap.begin()}; it != endIt; ++it)
        {
            bids.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
        }

        size_t i{};
        // for asks just go `levels` level from best ask
        for (auto it{askMap.begin()}; it != askMap.end(); ++it)
        {
            if (i >= levels) {break;}
            asks.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
            ++i;
        }

    } else if (price > bestAsk) // centered around an ask price
    {
        // logic here is basically the same
        auto endIt{askMap.begin()};
        while (endIt != askMap.end())
        {
            if (endIt->first * tickSize > price)
            {
                if (std::distance(endIt, askMap.end()) >= static_cast<int>(levels))
                {
                    std::advance(endIt, levels);
                } else
                {
                    endIt = askMap.end();
                }
            }
            ++endIt;
        }

        for (auto it{askMap.begin()}; it != endIt; ++it)
        {
            asks.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
        }

        size_t i{};
        for (auto it{bidMap.begin()}; it != bidMap.end(); ++it)
        {
            if (i >= levels) {break;}
            bids.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
            ++i;
        }

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
        for (auto it{askMap.begin()}; it != askMap.end(); ++it)
        {
            if (it->first * tickSize < minPrice) {continue;} // not yet reached min
            if (it->first * tickSize > maxPrice) {break;} // overshot max
            asks.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
        }

    } else if (minPrice <= bestBid && maxPrice <= bestBid)
    {
        for (auto it{bidMap.begin()}; it != bidMap.end(); ++it)
        {
            if (it->first * tickSize < minPrice) {continue;} // not yet reached min
            if (it->first * tickSize > maxPrice) {break;} // overshot max
            asks.emplace_back(it->first * tickSize, it->second.volume, it->second.orders.size());
        }

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
