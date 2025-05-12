// for catch testing
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include <iostream>

int test() {return 3;}

TEST_CASE("OrderBook")
{
    REQUIRE(test() == 3);
    REQUIRE_FALSE(test() == 5);
}

