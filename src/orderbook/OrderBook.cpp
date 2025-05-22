#include "orderbook/OrderBook.h"

bool OrderAudit::equals_to(const OrderAudit& other) const
{
    return id == other.id
    && volume_delta == other.volume_delta;
}

bool OrderBook::Level::operator==(const Level& other) const
{
    return price == other.price
    && volume == other.volume
    && orderCount == other.orderCount;
}

bool OrderBook::Depth::operator==(const Depth& other) const
{
    return bids == other.bids
    && asks == other.asks
    && volume == other.volume
    && bestBid == other.bestBid
    && bestAsk == other.bestAsk
    && marketPrice == other.marketPrice;
}

auto now() {return std::chrono::system_clock::now();}

OrderResult OrderBook::placeOrder(Order& order)
{
    // generate a unique id and timestamp for the order
    auto id{uuid_generator()};
    time_ ts{now()};

    // store in id pool for persistent storage and get pointer
    auto [it, _] = idPool.insert(id);

    // stamp the original order object first
    // OPTIMIZE: can skip stamping for rvalues since user wont keep
    order.id = &(*it);
    order.timestamp = ts;

    // bookkeeping
    orderList.push_back(order);

    // local active copy that is actually processed
    auto activeCopy{order};

    // match with existing orders and return result
    auto result{matchOrder(activeCopy)};

    // if order remains unfilled
    if (activeCopy.volume > 0)
    {
        // put in bid/ask map
        order_list::iterator activeItr;

        // some code duplication here since bid_map and ask_map are diff types
        if (activeCopy.side == Order::Side::BUY)
        {
            // add the order to the end of the orderlist
            // and update volume at PriceLevel
            auto& pLevel{bidMap[activeCopy.price]};
            pLevel.volume += activeCopy.volume;
            pLevel.orders.push_back(std::move(activeCopy));
            activeItr = std::prev(pLevel.orders.end());
        } else
        {
            auto& pLevel{askMap[activeCopy.price]};
            pLevel.volume += activeCopy.volume;
            pLevel.orders.push_back(std::move(activeCopy));
            activeItr = std::prev(pLevel.orders.end());
        }

        // add to idMap
        idMap[order.id] = OrderLocation{order.price, activeItr, order.side};
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

OrderResult OrderBook::modifyPrice(const uuids::uuid* id, float price)
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

OrderResult OrderBook::modifyPrice(const uuids::uuid& id, float price)
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

const order_list& OrderBook::bidsAt(float priceLevel)
{
    [[maybe_unused]] float lol = priceLevel * 2;
    return dummy;
}

const order_list& OrderBook::asksAt(float priceLevel)
{
    [[maybe_unused]] float lol = priceLevel * 2;
    return dummy;
}

const Order& OrderBook::getOrderByID(const uuids::uuid* id)
{
    return *idMap.at(id).itr;
}

const Order& OrderBook::getOrderByID(const uuids::uuid& id)
{
    [[maybe_unused]] auto lol = id;
    return *dummy.begin();
}

int OrderBook::volumeAt(float priceLevel)
{
    [[maybe_unused]] float lol = priceLevel * 2;
    return -1;
}

// center around best bid/ask
OrderBook::Depth OrderBook::getDepth(size_t levels)
{
    [[maybe_unused]] auto lol = levels * 2;
    return {std::vector<OrderBook::Level>(), std::vector<OrderBook::Level>(), 0, 0, 0, 0};
}

// center around a given price
OrderBook::Depth OrderBook::getDepthAtPrice(float price, size_t levels)
{
    [[maybe_unused]] auto lol = levels * 2;
    [[maybe_unused]] auto lol2 = price * 2;
    return {std::vector<OrderBook::Level>(), std::vector<OrderBook::Level>(), 0, 0, 0, 0};
}

// depth in a given range
OrderBook::Depth OrderBook::getDepthInRange(float maxPrice, float minPrice)
{
    [[maybe_unused]] auto lol = maxPrice * 2;
    [[maybe_unused]] auto lol2 = minPrice * 2;
    return {std::vector<OrderBook::Level>(), std::vector<OrderBook::Level>(), 0, 0, 0, 0};
}
