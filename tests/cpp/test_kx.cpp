#include <catch2/catch.hpp>

#include <kx/k.h>

TEST_CASE("REFERENCE_COUNTING")
{
    auto k_int = ki(5);
    // no additional copies
    REQUIRE(k_int->r == 0);
    // now one addition copes
    REQUIRE(r1(k_int) == k_int);
    REQUIRE(k_int->r == 1);
    // now no additional copies
    REQUIRE_NOTHROW(r0(k_int));
    REQUIRE(k_int->r == 0);
    // deallocated but ptr not nullptr
    REQUIRE_NOTHROW(r0(k_int));

    // Nullptr checks - both segfault
    //REQUIRE(r1(nullptr) == nullptr);
    //REQUIRE_NOTHROW(r0(nullptr));
}