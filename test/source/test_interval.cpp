#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <ldsgen/ilds.hpp>     // for Vdcorput
#include <list>                // for list, __list_iterator, operator!=
#include <ostream>             // for operator<<
#include <recti/interval.hpp>  // for Interval, operator<<, operator+, ope...
#include <set>                 // for set, set<>::iterator

using recti::hull;
using recti::Interval;

TEST_CASE("Interval test") {
    const auto interval_a = Interval{4, 8};
    const auto interval_b = Interval{5, 6};
    const auto value = 3;

    CHECK(!(interval_a < interval_b));
    CHECK(!(interval_b < interval_a));
    CHECK(!(interval_a > interval_b));
    CHECK(!(interval_b > interval_a));
    CHECK(interval_a <= interval_b);
    CHECK(interval_b <= interval_a);
    CHECK(interval_a >= interval_b);
    CHECK(interval_b >= interval_a);

    CHECK(!(interval_b == interval_a));
    CHECK(interval_b != interval_a);

    CHECK((interval_a - value) + value == interval_a);

    CHECK(interval_a.contains(4));
    CHECK(interval_a.contains(8));
    CHECK(interval_a.intersect_with(8) == Interval{8, 8});
    CHECK(interval_a.intersect_with(10).is_invalid());
    CHECK(interval_a.contains(interval_b));
    CHECK(interval_a.intersect_with(interval_b) == interval_b);
    CHECK(!interval_b.contains(interval_a));
    CHECK(interval_a.overlaps(interval_b));
    CHECK(interval_b.overlaps(interval_a));

    CHECK(min_dist(interval_a, interval_b) == 0);
}

TEST_CASE("Interval test hull") {
    const auto interval_a = Interval{3, 5};
    const auto interval_b = Interval{5, 7};
    const auto interval_c = Interval{7, 8};

    CHECK(interval_a.hull_with(interval_b) == Interval{3, 7});
    CHECK(interval_a.hull_with(interval_c) == Interval{3, 8});
    CHECK(interval_b.hull_with(interval_c) == Interval{5, 8});

    const auto value = 4;
    CHECK(interval_a.hull_with(value) == Interval{3, 5});
    CHECK(interval_a.hull_with(6) == Interval{3, 6});

    CHECK(hull(interval_a, value) == Interval{3, 5});
    CHECK(hull(interval_a, 6) == Interval{3, 6});
    CHECK(hull(value, interval_a) == Interval{3, 5});
    CHECK(hull(6, interval_a) == Interval{3, 6});
    CHECK(hull(value, 6) == Interval{4, 6});
}

TEST_CASE("Interval of Interval test") {
    const auto interval_a = Interval{Interval{3, 4}, Interval{8, 9}};
    const auto interval_b = Interval{Interval{5, 6}, Interval{6, 7}};
    const auto value = 3;

    CHECK(!(interval_a < interval_b));
    CHECK(!(interval_b < interval_a));
    CHECK(!(interval_a > interval_b));
    CHECK(!(interval_b > interval_a));
    CHECK(interval_a <= interval_b);
    CHECK(interval_b <= interval_a);
    CHECK(interval_a >= interval_b);
    CHECK(interval_b >= interval_a);

    CHECK(!(interval_b == interval_a));
    CHECK(interval_b != interval_a);

    CHECK((interval_a - value) + value == interval_a);

    CHECK(interval_a.contains(Interval{4, 5}));
    CHECK(interval_a.contains(Interval{7, 8}));

    CHECK(interval_a.intersect_with(Interval{7, 8}) == Interval{Interval{7, 7}, Interval{8, 8}});  // ???

    CHECK(interval_a.contains(interval_b));
    CHECK(interval_a.intersect_with(interval_b) == interval_b);
    CHECK(!interval_b.contains(interval_a));
    CHECK(interval_a.overlaps(interval_b));
    CHECK(interval_b.overlaps(interval_a));
}

TEST_CASE("Interval overlapping test") {
    constexpr auto max_count = 20;
    auto lst = std::list<Interval<int>>{};
    auto hgenX = ildsgen::VdCorput(3, 7);

    for (auto index_i = 0; index_i != max_count; ++index_i) {
        for (auto index_j = 0; index_j != max_count; ++index_j) {
            auto x_value = int(hgenX.pop());
            auto x_range = Interval<int>{x_value, x_value + 100};
            lst.push_back(x_range);
        }
    }

    std::set<Interval<int>> S;   // set of maximal non-overlapped rectangles
    std::list<Interval<int>> L;  // list of the removed rectangles

    for (const auto& intvl : lst) {
        if (S.contains(intvl)) {
            L.push_back(intvl);
        } else {
            S.insert(intvl);
        }
    }
}

TEST_CASE("Interval class") {
    Interval<int> interval_a(3, 4);
    SUBCASE("Constructor and accessors") {
        CHECK(interval_a.lb() == 3);
        CHECK(interval_a.ub() == 4);
    }
    SUBCASE("is_invalid") {
        CHECK(interval_a.is_invalid() == false);
        Interval<int> interval_b(4, 3);
        CHECK(interval_b.is_invalid() == true);
    }
    SUBCASE("measure") {
        CHECK(interval_a.measure() == 1);
        Interval<int> interval_b(3, 8);
        CHECK(interval_b.measure() == 5);
    }
    SUBCASE("nearest_to") {
        Interval<int> interval_c(3, 5);
        CHECK(interval_c.nearest_to(8) == 5);
        CHECK(interval_c.nearest_to(0) == 3);
        CHECK(interval_c.nearest_to(4) == 4);
    }
    SUBCASE("get_center") {
        Interval<int> interval_d(3, 7);
        CHECK(interval_d.get_center() == 5);
    }
    SUBCASE("lower_corner") {
        Interval<int> interval_e(3, 7);
        CHECK(interval_e.lower_corner() == 3);
    }
    SUBCASE("upper_corner") {
        Interval<int> interval_f(3, 7);
        CHECK(interval_f.upper_corner() == 7);
    }
    SUBCASE("enlarge_with") {
        Interval<int> interval_g(3, 5);
        auto enlarged = interval_g.enlarge_with(2);
        CHECK(enlarged.lb() == 1);
        CHECK(enlarged.ub() == 7);
    }
}