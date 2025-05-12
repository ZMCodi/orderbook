// for catch testing
// #define CATCH_CONFIG_MAIN
// #include <catch2/catch_all.hpp>

#include <iostream>
#include <array>
#include <string>
class Order
{
public:
    enum Side
    {
        BUY,
        SELL
    };
    static constexpr std::array<std::string, 2> side_str{"BUY", "SELL"};

    enum Type
    {
        LIMIT,
        MARKET
    };
    static constexpr std::array<std::string, 2> type_str{"LIMIT", "MARKET"};

    Order(Side side, int volume, Type type, float price)
    : side{side}
    , volume{volume}
    , type{type}
    , price{price} {}

    friend std::ostream& operator<<(std::ostream& out, Order order)
    {
        return out << "Order(SIDE:" << Order::side_str[order.side] << ", VOL:" << order.volume
            << ", TYPE:" << Order::type_str[order.type] << ", PRICE:" << order.price << ")";
    }

private:
    Side side;
    int volume;
    Type type;
    float price;

};

class Trade
{

};

class OrderBook
{

};

int main()
{
    Order order{Order::BUY, 3, Order::MARKET, 20};
    Order order2{Order::SELL, 5, Order::LIMIT, 50};
    std::cout << order << '\n' << order2 << std::endl;
}
