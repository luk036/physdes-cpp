#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <ldsgen/ilds.hpp>     // for VdCorput
#include <list>                // for list, __list_iterator, operator!=
#include <ostream>             // for operator<<
#include <recti/interval.hpp>  // for Interval, operator<<, operator+, ope...
#include <set>                 // for set, set<>::iterator

using recti::Interval;

TEST_CASE("Interval test") {
    auto a = Interval<int>{4, 8};
    auto b = Interval<int>{5, 6};
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
    CHECK(a.intersection_with(8) == Interval<int>{8, 8});
    CHECK(a.contains(b));
    CHECK(a.intersection_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));

    CHECK(min_dist(a, b) == 0);
}

TEST_CASE("Interval of Interval test") {
    auto a = Interval<Interval<int>>{Interval<int>{3, 4}, Interval<int>{8, 9}};
    auto b = Interval<Interval<int>>{Interval<int>{5, 6}, Interval<int>{6, 7}};
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

    CHECK(a.contains(Interval<int>{4, 5}));
    CHECK(a.contains(Interval<int>{7, 8}));

    CHECK(a.intersection_with(Interval<int>{7, 8})
          == Interval<Interval<int>>{Interval<int>{7, 7}, Interval<int>{8, 8}});  // ???

    CHECK(a.contains(b));
    CHECK(a.intersection_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));
}

TEST_CASE("Interval overlapping test") {
    constexpr auto N = 20;
    auto lst = std::list<Interval<int>>{};
    auto hgenX = ildsgen::VdCorput(3, 7);

    for (auto i = 0; i != N; ++i) {
        for (auto j = 0; j != N; ++j) {
            auto x = int(hgenX.pop());
            auto xrng = Interval<int>{x, x + 100};
            lst.push_back(xrng);
        }
    }

    std::set<Interval<int>> S;   // set of maximal non-overlapped rectangles
    std::list<Interval<int>> L;  // list of the removed rectangles

    for (const auto &intvl : lst) {
        if (S.contains(intvl)) {
            L.push_back(intvl);
        } else {
            S.insert(intvl);
        }
    }
}
