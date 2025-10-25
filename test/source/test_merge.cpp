#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST...

#include <iostream>              // for operator<<, ostream, basic_ostream
#include <recti/halton_int.hpp>  // for recti
#include <recti/manhattan_arc.hpp>   // for ManhattanArc, operator+, operator-

#include "recti/interval.hpp"  // for min_dist, overlap, interval

using namespace recti;

TEST_CASE("ManhattanArc test") {
    const auto r1 = ManhattanArc<int>{4 - 5, 4 + 5};
    const auto r2 = ManhattanArc<int>{7 - 9, 7 + 9};

    CHECK_EQ(r1, r1);
    CHECK(r1 != r2);
    CHECK(!overlap(r1, r2));
    CHECK(r1.min_dist_with(r2) == 7);
    CHECK(min_dist(r1, r2) == 7);
}

TEST_CASE("ManhattanArc class") {
    ManhattanArc<int, int> a(4 - 5, 4 + 5); // [-1, 9]
    SUBCASE("Equality") {
        ManhattanArc<int, int> b(7 - 9, 7 + 9); // [-2, 16]
        CHECK(a == a);
        CHECK_FALSE(a == b);
        ManhattanArc<int, int> c(-1, 9);
        CHECK(a == c);
    }
    SUBCASE("min_dist_with") {
        ManhattanArc<int, int> b(7 - 9, 7 + 9); // [-2, 16]
        CHECK(a.min_dist_with(b) == 7);
    }
    SUBCASE("get_center") {
        auto center = a.get_center();
        CHECK(center.xcoord() == 4);
        CHECK(center.ycoord() == 5);
    }
    SUBCASE("get_lower_corner") {
        auto lower = a.get_lower_corner();
        CHECK(lower.xcoord() == 4);
        CHECK(lower.ycoord() == 5);
    }
    SUBCASE("get_upper_corner") {
        auto upper = a.get_upper_corner();
        CHECK(upper.xcoord() == 4);
        CHECK(upper.ycoord() == 5);
    }
    SUBCASE("nearest_point_to") {
        Point<int, int> p(0, 0);
        auto nearest = a.nearest_point_to(p);
        CHECK(nearest.xcoord() == 4);
        CHECK(nearest.ycoord() == 5);
    }
    SUBCASE("merge_with") {
        ManhattanArc<int, int> b(7 - 9, 7 + 9); // [-2, 16]
        auto merged = a.merge_with(b, 3);
        // Expected result: /[-4, 2], [12, 12]/
        CHECK(merged.get_lower_corner().xcoord() == 4);
    }
}
