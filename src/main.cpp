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

// inline std::ostream& operator<<(std::ostream& out, const Order& o)
// {
//     std::stringstream str;
//     str << "Order(id: " << *o.id << ", side: ";

//     if (o.side == Order::Side::BUY)
//     {
//         str << "BUY, ";
//     } else if (o.side == Order::Side::SELL)
//     {
//         str << "SELL, ";
//     }

//     str << "vol: " << o.volume << ", type: ";

//     if (o.type == Order::Type::LIMIT)
//     {
//         str << "LIMIT, ";
//     } else if (o.type == Order::Type::MARKET)
//     {
//         str << "MARKET, ";
//     }

//     str << "price: " << std::fixed << o.price << ", timestamp: "
//     << o.timestamp << ")";

//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, const TradeCopy& t)
// {
//     std::stringstream str;
//     str << "Trade(id: " << t.id << ", buyer_id: " << t.buyer_id
//     << ", seller_id: " << t.seller_id << ", price: " << t.price
//     << ", volume: " << t.volume << ", timestamp: " << t.timestamp
//     << ", taker: ";

//     switch (t.taker)
//     {
//         case Order::Side::BUY:
//             str << "BUY";
//             break;
//         case Order::Side::SELL:
//             str << "SELL";
//             break;
//     }

//     str << ")";
//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, const Trade& t)
// {
//     return out << TradeCopy{t};
// }

// inline std::ostream& operator<<(std::ostream& out, const trades& t)
// {
//    std::stringstream str;
//    str << "[";

//    for (size_t i = 0; i < t.size(); ++i) {
//        str << t[i];
//        if (i < t.size() - 1) {
//            str << ", ";
//        }
//    }

//    str << "]";
//    out << str.str();
//    return out;
// }

// inline std::ostream& operator<<(std::ostream& out, const OrderResult& o)
// {
//     std::stringstream str;
//     str << "OrderResult(id: " << o.order_id << ", status: ";

//     switch (o.status)
//     {
//     case OrderResult::PLACED:
//         str << "PLACED";
//         break;
//     case OrderResult::FILLED:
//         str << "FILLED";
//         break;
//     case OrderResult::PARTIALLY_FILLED:
//         str << "PARTIALLY_FILLED";
//         break;
//     case OrderResult::REJECTED:
//         str << "REJECTED";
//         break;
//     case OrderResult::MODIFIED:
//         str << "MODIFIED";
//         break;
//     case OrderResult::CANCELLED:
//         str << "CANCELLED";
//         break;
//     }

//     str << ", trades: " << o.trades << ", remainingOrder: "
//     << o.remainingOrder << ", message: " << o.message << ")";

//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, order_list olist)
// {
//     std::stringstream str;
//     str << "[";

//     for (auto itr{olist.begin()}; itr != olist.end(); ++itr) {
//         str << *itr;
//         str << ", ";
//     }

//     str << "]";
//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, PriceLevel p)
// {
//     std::stringstream str;
//     str << "PriceLevel(volume: " << p.volume << ", orders: " << p.orders << ")";
//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, ask_map a)
// {
//     std::stringstream str;
//     str << "{\n";

//     for (auto it{a.begin()}; it != a.end(); ++it)
//     {
//         str << '\t' << it->first << ": " << it->second << ",\n";
//     }

//     str << "\n}";
//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, bid_map a)
// {
//     std::stringstream str;
//     str << "{\n";

//     for (auto it{a.begin()}; it != a.end(); ++it)
//     {
//         str << '\t' << it->first << ": " << it->second << ",\n";
//     }

//     str << "\n}";
//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, OrderBook::Level l)
// {
//     std::stringstream str;
//     str << "Level(price: " << l.price << ", volume: " << l.volume
//     << ", orderCount: " << l.orderCount << ")";

//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, OrderBook::Depth d)
// {
//     std::stringstream str;
//     str << "Depth(\n\tbids: [\n\t";

//     for (auto bid : d.bids)
//     {
//         str << '\t' << bid << ",\n\t";
//     }

//     str << "],\n\tasks: [\n\t";

//     for (auto ask : d.asks)
//     {
//         str << '\t' << ask << ",\n\t";
//     }

//     str << "],\n\tvolume: " << d.volume << ", bestBid: " << d.bestBid
//     << ", bestAsk: " << d.bestAsk << ", marketPrice: " << d.marketPrice
//     << "\n)";

//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, OrderLocation o)
// {
//     std::stringstream str;
//     str << "OrderLocation(order: " << *o.itr << ", price: "
//     << o.price << ", side: ";

//     if (o.side == Order::Side::BUY) {str << "BUY)";}
//     if (o.side == Order::Side::SELL) {str << "SELL)";}

//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, id_map i)
// {
//      std::stringstream str;
//     str << "{\n";

//     for (auto it{i.begin()}; it != i.end(); ++it)
//     {
//         str << '\t' << it->first << ": " << it->second << ",\n";
//     }

//     str << "\n}";

//     return out << str.str();
// }

// inline std::ostream& operator<<(std::ostream& out, trade_list t)
// {
//     std::stringstream str;
//     str << "[";

//     for (size_t i = 0; i < t.size(); ++i) {
//         str << t[i];
//         if (i < t.size() - 1) {
//             str << ", ";
//         }
//     }

//     str << "]";
//     out << str.str();
//     return out;
// }

// inline std::ostream& operator<<(std::ostream& out, orders o)
// {
//     std::stringstream str;
//     str << "[";

//     for (size_t i = 0; i < o.size(); ++i) {
//         str << o[i];
//         if (i < o.size() - 1) {
//             str << ", ";
//         }
//     }

//     str << "]";
//     out << str.str();
//     return out;
// }

// inline std::ostream& operator<<(std::ostream& out, OrderBookState s)
// {
//     std::stringstream str;
//     str << "State(\n";
//     str << "\tbidMap: " << s.bidMap << ",\n";
//     str << "\taskMap: " << s.askMap << ",\n";
//     str << "\tidMap: " << s.idMap << ",\n";
//     str << "\ttradeList: " << s.tradeList << ",\n";
//     str << "\torderList: " << s.orderList << ",\n";
//     str << "\tbestBid: " << s.bestBid
//     << ", bestAsk: " << s.bestAsk
//     << ", marketPrice: " << s.marketPrice
//     << ", totalVolume: " << s.totalVolume << "\n)";

//     return out << str.str();
// }

#include <fstream>
int main()
{
    OrderBook ob{};
    Order buy50{Order::Side::BUY, 2, Order::Type::LIMIT, 50};

    std::cout << ob.placeOrder(buy50);
}
