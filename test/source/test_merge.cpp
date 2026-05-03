#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST...

#include <iostream>                 // for operator<<, ostream, basic_ostream
// #include <recti/halton_int.hpp>     // for recti
#include <recti/manhattan_arc.hpp>  // for ManhattanArc, operator+, operator-

// #include "recti/interval.hpp"  // for min_dist, overlap, interval

using namespace recti;

TEST_CASE("ManhattanArc test") {
    const auto r1 = ManhattanArc<int>{4 - 5, 4 + 5};
    const auto r2 = ManhattanArc<int>{7 - 9, 7 + 9};

    CHECK_EQ(r1, r1);
    CHECK_NE(r1, r2);
    CHECK_FALSE(overlap(r1, r2));
    CHECK_EQ(r1.min_dist_with(r2), 7);
    CHECK_EQ(min_dist(r1, r2), 7);
}

TEST_CASE("ManhattanArc class") {
    ManhattanArc<int, int> a(4 - 5, 4 + 5);  // [-1, 9]
    SUBCASE("Equality") {
        ManhattanArc<int, int> b(7 - 9, 7 + 9);  // [-2, 16]
        CHECK_EQ(a, a);
        CHECK_FALSE(a == b);
        ManhattanArc<int, int> c(-1, 9);
        CHECK_EQ(a, c);
    }
    SUBCASE("min_dist_with") {
        ManhattanArc<int, int> b(7 - 9, 7 + 9);  // [-2, 16]
        CHECK_EQ(a.min_dist_with(b), 7);
    }
    SUBCASE("get_center") {
        auto center = a.get_center();
        CHECK_EQ(center.xcoord(), 4);
        CHECK_EQ(center.ycoord(), 5);
    }
    SUBCASE("get_lower_corner") {
        auto lower = a.get_lower_corner();
        CHECK_EQ(lower.xcoord(), 4);
        CHECK_EQ(lower.ycoord(), 5);
    }
    SUBCASE("get_upper_corner") {
        auto upper = a.get_upper_corner();
        CHECK_EQ(upper.xcoord(), 4);
        CHECK_EQ(upper.ycoord(), 5);
    }
    SUBCASE("nearest_point_to") {
        Point<int, int> p(0, 0);
        auto nearest = a.nearest_point_to(p);
        CHECK_EQ(nearest.xcoord(), 4);
        CHECK_EQ(nearest.ycoord(), 5);
    }
    SUBCASE("merge_with") {
        ManhattanArc<int, int> b(7 - 9, 7 + 9);  // [-2, 16]
        auto merged = a.merge_with(b, 3);
        // Expected result: /[-4, 2], [12, 12]/
        CHECK_EQ(merged.get_lower_corner().xcoord(), 4);
    }
}
