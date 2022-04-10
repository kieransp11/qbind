#include <catch2/catch.hpp>

#include <kx/k.h>

#include "qbind/ref.h"
#include "qbind/type.h"
#include "qbind/span.h"
#include "qbind/converter.h"

/**
 * @brief Test that atoms and singleton lists can be used interchangably.
 * 
 */
TEST_CASE("SPAN_ATOM_SINGLETON_SEMANTICS")
{
    qbind::Converter c;

    SECTION("ATOM_AS_ATOM")
    {
        REQUIRE(99 == c.to_cpp<int>(ki(99)));
    }

    SECTION("ATOM_AS_SINGLETON")
    {
        auto span = c.to_cpp<qbind::Span<qbind::Integer>>(ki(99));
        REQUIRE(span.size() == 1);
        REQUIRE(*span.front() == 99);
        REQUIRE(*span.back() == 99);
        REQUIRE(*span[0] == 99);
        span[0] = 88;
        REQUIRE(span.size() == 1);
        REQUIRE(*span.front() == 88);
        REQUIRE(*span.back() == 88);
        REQUIRE(*span[0] == 88);
    }

    SECTION("SINGLETON_AS_ATOM")
    {
        auto arr = ktn(KI, 1);
        ((I *)(arr->G0))[0] = 777;
        REQUIRE(777 == c.to_cpp<int>(arr));
    }

    SECTION("SINGLETON_AS_SINGLETON")
    {
        auto arr = ktn(KI, 1);
        ((I *)(arr->G0))[0] = 777;
        auto span = c.to_cpp<qbind::Span<qbind::Integer>>(arr);
        REQUIRE(span.size() == 1);
        REQUIRE(*span.front() == 777);
        REQUIRE(*span.back() == 777);
        REQUIRE(*span[0] == 777);
        span[0] = 666;
        REQUIRE(span.size() == 1);
        REQUIRE(*span.front() == 666);
        REQUIRE(*span.back() == 666);
        REQUIRE(*span[0] == 666);
    }
}

TEST_CASE("ELEMENT_ACCESSORS")
{
    const auto length = 10;
    auto arr = ktn(KI, length);
    for (auto i = 0; i < length; ++i)
    {
        ((I *)(arr->G0))[i] = (i+1)*111;
    }

    qbind::Converter c;
    auto span = c.to_cpp<qbind::Span<qbind::Integer>>(arr);

    REQUIRE(*span.front() == 111);
    REQUIRE(*span.begin() == 111);
    REQUIRE(*span.cbegin() == 111);
    REQUIRE(span[0] == 111);
    REQUIRE(span.at(0) == 111);

    REQUIRE(*span.back() == 1110);
    REQUIRE(*span.rbegin() == 1110);
    REQUIRE(*span.crbegin() == 1110);
    REQUIRE(span[length - 1] == 1110);
    REQUIRE(span.at(length - 1) == 1110);

    REQUIRE(span.end() - span.begin() == length);
    REQUIRE(span.cend() - span.cbegin() == length);
    REQUIRE(span.rbegin() - span.rend() == length);
    REQUIRE(span.crbegin() - span.crend() == length);

    auto start = span.first(5);
    REQUIRE(start[0] == 111);
    REQUIRE(start[4] == 555);

    auto end = span.last(5);
    REQUIRE(end[0] == 666);
    REQUIRE(end[4] == 1110);

    auto middle = span.subspan(3, 5);
    REQUIRE(middle[0] == 444);
    REQUIRE(middle[4] == 888);

    auto idx = 1;
    for (const auto &x : span)
    {
        REQUIRE(111 * idx == x);
        ++idx;
    }
}

TEST_CASE("BASIC_TUPLE")
{
    qbind::Converter c;

    SECTION("ATOMS_AS_ATOMS")
    {
        auto mixedk = knk(2, ki(99), kj(88));
        auto [intres, longres] = c.to_cpp<std::tuple<int32_t, int64_t>>(mixedk);
        REQUIRE(intres == 99);
        REQUIRE(longres == 88);
    }

    SECTION("ATOMS_AS_SINGLETONS")
    {
        auto mixedk = knk(2, ki(99), kj(88));
        auto [intspan, longspan] = c.to_cpp<
            std::tuple<qbind::Span<qbind::Integer>, qbind::Span<qbind::Long>>>(mixedk);

        REQUIRE(*intspan.front() == 99);
        REQUIRE(*longspan.front() == 88);
    }

    SECTION("SINGLETON_AS_ATOMS")
    {
        auto iarr = ktn(KI, 1);
        ((I *)(iarr->G0))[0] = 777;
        auto jarr = ktn(KJ, 1);
        ((J *)(jarr->G0))[0] = 888;
        auto mixedk = knk(2, iarr, jarr);
        auto [intres, longres] = c.to_cpp<std::tuple<int32_t, int64_t>>(mixedk);
        REQUIRE(intres == 777);
        REQUIRE(longres == 888);
    }

    SECTION("SINGLETON_AS_ATOMS")
    {
        auto iarr = ktn(KI, 1);
        ((I *)(iarr->G0))[0] = 777;
        auto jarr = ktn(KJ, 1);
        ((J *)(jarr->G0))[0] = 888;
        auto mixedk = knk(2, iarr, jarr);
        auto [intspan, longspan] = c.to_cpp<
            std::tuple<qbind::Span<qbind::Integer>, qbind::Span<qbind::Long>>>(mixedk);
        REQUIRE(*intspan.front() == 777);
        REQUIRE(*longspan.front() == 888);
    }

}