#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <ldsgen/ilds.hpp>     // for Vdcorput
#include <list>                // for list, __list_iterator, operator!=
#include <ostream>             // for operator<<
#include <recti/interval.hpp>  // for Interval, operator<<, operator+, ope...
#include <set>                 // for set, set<>::iterator

using recti::hull;
using recti::Interval;

TEST_CASE("Interval test") {
    const auto a = Interval{4, 8};
    const auto b = Interval{5, 6};
    const auto v = 3;

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
    CHECK(a.intersect_with(8) == Interval{8, 8});
    CHECK(a.intersect_with(10).is_invalid());
    CHECK(a.contains(b));
    CHECK(a.intersect_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));

    CHECK(min_dist(a, b) == 0);
}

TEST_CASE("Interval test hull") {
    const auto a = Interval{3, 5};
    const auto b = Interval{5, 7};
    const auto c = Interval{7, 8};

    CHECK(a.hull_with(b) == Interval{3, 7});
    CHECK(a.hull_with(c) == Interval{3, 8});
    CHECK(b.hull_with(c) == Interval{5, 8});

    const auto d = 4;
    CHECK(a.hull_with(d) == Interval{3, 5});
    CHECK(a.hull_with(6) == Interval{3, 6});

    CHECK(hull(a, d) == Interval{3, 5});
    CHECK(hull(a, 6) == Interval{3, 6});
    CHECK(hull(d, a) == Interval{3, 5});
    CHECK(hull(6, a) == Interval{3, 6});
    CHECK(hull(d, 6) == Interval{4, 6});
}

TEST_CASE("Interval of Interval test") {
    const auto a = Interval{Interval{3, 4}, Interval{8, 9}};
    const auto b = Interval{Interval{5, 6}, Interval{6, 7}};
    const auto v = 3;

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

    CHECK(a.contains(Interval{4, 5}));
    CHECK(a.contains(Interval{7, 8}));

    CHECK(a.intersect_with(Interval{7, 8}) == Interval{Interval{7, 7}, Interval{8, 8}});  // ???

    CHECK(a.contains(b));
    CHECK(a.intersect_with(b) == b);
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
