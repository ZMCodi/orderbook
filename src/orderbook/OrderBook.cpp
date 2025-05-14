#include "orderbook/OrderBook.h"

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

OrderResult OrderBook::place_order(Order& order)
{
    return {order.id, OrderResult::FILLED, std::vector<Trade>(), &order, ""};
}

const std::list<Order>& OrderBook::bidsAt(float priceLevel)
{
    [[maybe_unused]] float lol = priceLevel * 2;
    return dummy;
}

const std::list<Order>& OrderBook::asksAt(float priceLevel)
{
    [[maybe_unused]] float lol = priceLevel * 2;
    return dummy;
}

const Order& OrderBook::getOrderByID(std::string_view id)
{
    [[maybe_unused]] size_t lol = id.size();
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
