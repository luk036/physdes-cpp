#include <doctest/doctest.h>
// #include <random>
// #include <iostream>
#include <cstdlib>
#include <list>
#include <recti/recti.hpp>
#include <set>

// using std::randint;
using namespace recti;

// template <typename T> struct my_point : point<T, T> { double data; };

TEST_CASE("Point test") {
    auto a = point<int>{4, 8};
    auto b = point<int>{5, 6};
    auto v = (b - a) / 2;  // integer division

    CHECK(a < b);
    CHECK(a <= b);
    CHECK(!(a == b));
    CHECK(a != b);
    CHECK(b > a);
    CHECK(b >= a);
    CHECK((a + v) + v != b);  // due to integer division
    CHECK((a - v) + v == a);

    CHECK(a.flip_xy().flip_xy() == a);
    CHECK(a.flip_y().flip_y() == a);
}

TEST_CASE("Interval test") {
    auto a = interval{4, 8};
    auto b = interval{5, 6};
    auto v = 3;

    CHECK(!(a < b));
    CHECK(!(b < a));
    CHECK(!(a > b));
    CHECK(!(b > a));
    CHECK(a <= b);
    CHECK(b <= a);
    CHECK(a >= b);
    CHECK(b >= a);

    CHECK(!(b == a));
    CHECK(b != a);

    CHECK((a - v) + v == a);

    CHECK(a.contains(4));
    CHECK(a.contains(8));
    CHECK(a.contains(b));
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));
}

TEST_CASE("Rectangle test") {
    auto xrng1 = interval{4, 8};
    auto yrng1 = interval{5, 7};
    auto r1 = rectangle{xrng1, yrng1};
    auto xrng2 = interval{5, 7};
    auto yrng2 = interval{6, 6};
    auto r2 = rectangle{xrng2, yrng2};
    auto p = point{7, 6};
    auto v = vector2{5, 6};

    CHECK(r1 != r2);
    CHECK((r1 - v) + v == r1);

    CHECK(r1.contains(p));
    CHECK(r1.contains(r2));
    CHECK(r1.overlaps(r2));
}

TEST_CASE("Segment test") {
    auto xrng1 = interval{4, 8};
    auto yrng1 = interval{5, 7};
    auto s1 = hsegment{xrng1, 6};
    auto s2 = vsegment{5, yrng1};

    CHECK(s1.overlaps(s2));
}

TEST_CASE("Interval overlapping test") {
    constexpr auto N = 20;
    auto lst = std::list<interval<int>>{};

    for (auto i = 0; i != N; ++i) {
        int ii = i * 100;
        auto intvl = interval{ii, ii + std::rand() % 100};
        lst.push_back(intvl);
    }

    std::set<interval<int>> S;   // set of maximal non-overlapped intervals
    std::list<interval<int>> L;  // list of the removed intervals

    for (const auto& intvl : lst) {
        if (S.find(intvl) != S.end()) {
            L.push_back(intvl);
        } else {
            S.insert(intvl);
        }
    }

    // for (const auto& r : S)
    // {
    //     cout << "  \\draw " << r << ";\n";
    // }
    // for (const auto& r : L)
    // {
    //     cout << "  \\draw[color=red] " << r << ";\n";
    // }
}
