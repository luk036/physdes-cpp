#include <recti/generic.hpp>
#include <recti/interval.hpp>

#include "doctest/doctest.h"

struct MyPoint {
    int x, y;
    constexpr auto operator==(const MyPoint& other) const -> bool {
        return x == other.x && y == other.y;
    }
};

struct MyRect {
    MyPoint min, max;

    constexpr auto overlaps(const MyRect& other) const -> bool {
        return min.x < other.max.x && other.min.x < max.x && min.y < other.max.y
               && other.min.y < max.y;
    }

    constexpr auto contains(const MyPoint& p) const -> bool {
        return min.x <= p.x && p.x < max.x && min.y <= p.y && p.y < max.y;
    }
};

TEST_CASE("Generic functions") {
    const MyPoint p1{1, 2};
    const MyPoint p2{1, 2};
    const MyPoint p3{2, 3};

    CHECK(recti::overlap(p1, p2));
    CHECK(!recti::overlap(p1, p3));

    const MyRect r1{{0, 0}, {4, 4}};
    const MyRect r2{{2, 2}, {6, 6}};
    const MyRect r3{{8, 8}, {10, 10}};

    CHECK(recti::overlap(r1, r2));
    CHECK(!recti::overlap(r1, r3));

    CHECK(recti::contain(r1, p1));
    CHECK(recti::contain(r1, p3));
}

using namespace recti;

TEST_CASE("min_dist function") {
    // Scalar tests
    CHECK(min_dist(1, 3) == 2);
    CHECK(min_dist(3, 1) == 2);
    CHECK(min_dist(1, 1) == 0);
    // Interval tests
    Interval<int> a(3, 5);
    Interval<int> b(7, 9);
    CHECK(min_dist(a, 4) == 0); // 4 is within [3,5]
    CHECK(min_dist(a, 6) == 1); // Distance to upper bound
    CHECK(min_dist(a, b) == 2); // Distance between [3,5] and [7,9]
}

TEST_CASE("nearest function") {
    // Scalar tests
    CHECK(nearest(1, 1) == 1);
    CHECK(nearest(1, 3) == 1);
    // Interval tests
    Interval<int> a(1, 2);
    CHECK(nearest(a, 4) == 2); // Nearest to upper bound
    Interval<int> b(1, 5);
    CHECK(nearest(b, 4) == 4); // 4 is within [1,5]
}

TEST_CASE("measure_of function") {
    // Scalar test
    CHECK(measure_of(1) == 1);
    // Interval test
    Interval<int> a(1, 2);
    CHECK(measure_of(a) == 1);
}

TEST_CASE("center function") {
    // Scalar test
    CHECK(center(1) == 1);
    // Interval test
    Interval<int> a(1, 3);
    CHECK(center(a) == 2);
}

TEST_CASE("lower function") {
    // Scalar test
    CHECK(lower(1) == 1);
    // Interval test
    Interval<int> a(1, 3);
    CHECK(lower(a) == 1);
}

TEST_CASE("upper function") {
    // Scalar test
    CHECK(upper(1) == 1);
    // Interval test
    Interval<int> a(1, 3);
    CHECK(upper(a) == 3);
}
