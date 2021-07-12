/*
 *  Distributed under the MIT License (See accompanying file /LICENSE )
 */
#include "recti/fractions.hpp"
// #include <boost/multiprecision/cpp_int.hpp>
#include <doctest/doctest.h>
#include <iostream>

using namespace fun;

TEST_CASE("GCD")
{
    CHECK(gcd(0, 0) == 0);
    CHECK(gcd(1, 0) == 1);
    CHECK(gcd(0, 1) == 1);
    CHECK(gcd(-1, 0) == 1);
    CHECK(gcd(0, -1) == 1);
    CHECK(lcm(0, 0) == 0);
    CHECK(lcm(1, 0) == 0);
    CHECK(lcm(0, 1) == 0);
}

TEST_CASE("Fraction")
{
    // using boost::multiprecision::cpp_int;
    // static_assert(Integral<cpp_int>);
    const auto a = 3;
    const auto b = 4;
    const auto c = 5;
    const auto d = 6;
    // const auto f = -30;
    // const auto g = 4;
    // const auto z = 0;
    // const auto h = -g;

    auto p = Fraction {a, b};
    std::cout << p << '\n';
    const auto q = Fraction {c, d};

    CHECK(p == Fraction(30, 40));
    CHECK(2 * p == Fraction(6, 4));
    p *= 2;
    CHECK(p == Fraction(6, 4));
    CHECK(p / 2 == Fraction(30, 40));

    p /= 2;
    CHECK(p + q == Fraction(19, 12));
    CHECK(p - q == Fraction(-1, 12));
    CHECK(p < q);
    CHECK(p != q);
    CHECK(0 < p);
}

TEST_CASE("Fraction Special Cases")
{
    const auto p = Fraction {3, 4};
    const auto inf = Fraction {1, 0};
    // const auto nan = Fraction {0, 0};
    const auto zero = Fraction {0, 1};

    CHECK(-inf < zero);
    CHECK(zero < inf);
    CHECK(-inf < p);
    CHECK(p < inf);
    CHECK(inf == inf);
    CHECK(-inf < inf);
    CHECK(inf == inf * p);
    CHECK(inf == inf * inf);
    CHECK(inf == p / zero);
    CHECK(inf == inf / zero);
    // CHECK(nan == nan);
    // CHECK(nan == inf * zero);
    // CHECK(nan == -inf * zero);
    // CHECK(nan == inf / inf);
    // CHECK(nan == nan * zero);
    // CHECK(nan == nan * nan);
    // CHECK(nan == inf - inf);
    CHECK(inf == inf + inf);
    CHECK(inf + p == inf);   // ???
    CHECK(-inf + p == -inf); // ???
    CHECK(p + zero == p);
}
