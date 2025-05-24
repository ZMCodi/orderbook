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

std::ostream& operator<<(std::ostream& out, const Order& o)
{
    std::stringstream str;
    str << "Order(id: " << o.id << ", side: ";

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

    str << "price: " << o.price << ", timestamp: "
    << o.timestamp << ")";

    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, const Trade& t)
{
    std::stringstream str;
    str << "Trade(id: " << *t.id << ", buyer_id: " << *t.buyer_id
    << ", seller_id: " << *t.seller_id << ", price: " << t.price
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

std::ostream& operator<<(std::ostream& out, const trades& t)
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

std::ostream& operator<<(std::ostream& out, const OrderResult& o)
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

    str << ", trades: " << o.trades << ", remainingOrder: "
    << o.remainingOrder << ", message: " << o.message << ")";

    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, order_list olist)
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

std::ostream& operator<<(std::ostream& out, PriceLevel p)
{
    std::stringstream str;
    str << "PriceLevel(volume: " << p.volume << ", orders: " << p.orders << ")";
    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, ask_map a)
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

std::ostream& operator<<(std::ostream& out, bid_map a)
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

std::ostream& operator<<(std::ostream& out, OrderBook::Level l)
{
    std::stringstream str;
    str << "Level(price: " << l.price << ", volume: " << l.volume
    << ", orderCount: " << l.orderCount << ")";

    return out << str.str();
}

std::ostream& operator<<(std::ostream& out, OrderBook::Depth d)
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

#include <fstream>
int main()
{
    // OrderBook ob{};

    // for (int i = 0; i < 1000; ++i)
    // {
    //     // make the price differences granular
    //     ob.placeOrder(Order::makeLimitBuy(1, 59.95 - i * 0.01));
    //     ob.placeOrder(Order::makeLimitSell(1, 60.05 + i * 0.01));
    // }

    for (int i = 0; i < 10; ++i) {
        double askPrice = 60.05 + i * 0.01;
        std::cout << "i=" << i << ", askPrice=" << askPrice 
                << ", truncated=" << utils::trunc(askPrice, 0.01)
                << ", tick=" << utils::convertTick(askPrice, 0.01) << std::endl;
    }

    // for (int i = 0; i < 5; ++i) {
    //     double askPrice = 60.05 + i * 0.01;
    //     double divided = askPrice / 0.01 + 0.01;
    //     double floored = std::floor(divided);
    //     double result = floored * 0.01;
        
    //     std::cout << std::fixed << std::setprecision(20) 
    //             << "i=" << i << ", askPrice=" << askPrice 
    //             << ", divided=" << divided
    //             << ", floored=" << floored 
    //             << ", result=" << result << std::endl;
    // }


    // std::ofstream out{"depthMain.txt"};
    // std::cout.rdbuf(out.rdbuf());
    // std::cout << ob.getDepth(1000);
}
