/*
 *  Distributed under the MIT License (See accompanying file /LICENSE )
 */
#include <doctest/doctest.h>

#include <recti/vector2.hpp>

using namespace recti;

TEST_CASE("vector2") {
    // using boost::multiprecision::cpp_int;
    // static_assert(Integral<cpp_int>);
    const auto a = 3;
    const auto b = 4;
    const auto c = 5;
    const auto d = 6;

    const auto p = vector2{a, b};
    const auto q = vector2{c, d};

    CHECK(vector2{8, 10} == (p + q));
    CHECK(vector2{8, 2} != (p + q));
    CHECK(vector2{-2, -2} == (p - q));
    CHECK(vector2{6, 8} == (p * 2));
    CHECK(vector2{4, 5} == (p + q) / 2);

    CHECK(p != q);
    CHECK(p + q == q + p);
    CHECK(p - q == -(q - p));
    CHECK(p * 3 == 3 * p);
    CHECK(p + (q - p) / 2 == (p + q) / 2);  // ???

    const auto r = vector2{-b, c};
    CHECK((p + q) + r == p + (q + r));
}
