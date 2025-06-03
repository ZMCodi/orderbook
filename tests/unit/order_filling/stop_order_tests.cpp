#include <catch2/catch_all.hpp>

#include "test_helpers.h"

// TEST_CASE("Stop orders", "[order filling][stop]")
// {
//     OrderBook ob{};
//     Order SB55{Order::makeStopBuy(5, 55)};
//     Order SLB50_55{Order::makeStopLimitBuy(5, 50, 55)};

//     Order SS50{Order::makeStopSell(5, 50)};
//     Order SLS45_50{Order::makeStopLimitSell(5, 45, 50)};
    
//     SECTION("Stop orders go to stop map and not bid/ask map")
//     {
//         ob.placeOrder(SB55);
//         ob.placeOrder(SLB50_55);
//         ob.placeOrder(SS50);
//         ob.placeOrder(SLS45_50);

//         auto actual{ob.getState().stopMap};

//         stop_map expSM{
//             {5500, {0, order_list{SB55, SLB50_55}}},
//             {5000, {0, order_list{SS50, SLS45_50}}}
//         };

//         id_map expIDM{
//             {SB55.id, OrderLocation{55, expSM.at(5500).orders.begin(), OrderLocation::STOP}},
//             {SB55.id, OrderLocation{55, ++expSM.at(5500).orders.begin(), OrderLocation::STOP}},
//             {SB55.id, OrderLocation{55, expSM.at(5000).orders.begin(), OrderLocation::STOP}},
//             {SB55.id, OrderLocation{55, ++expSM.at(5000).orders.begin(), OrderLocation::STOP}},
//         };

//         OrderBookState expState{
//             bid_map(), ask_map(), expSM, expIDM, trade_list(),
//             orders{SB55, SLB50_55, SS50, SLS45_50},
//             -1, -1, -1, 0
//         };

//         REQUIRE(checkOBState(ob, expState));
//     }

//     SECTION(""){}
// }
