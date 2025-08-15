#include <recti/generic.hpp>

#include "doctest/doctest.h"

struct MyPoint {
    int x, y;
    constexpr auto operator==(const MyPoint& other) const -> bool { return x == other.x && y == other.y; }
};

struct MyRect {
    MyPoint min, max;

    constexpr auto overlaps(const MyRect& other) const -> bool {
        return min.x < other.max.x && other.min.x < max.x && min.y < other.max.y && other.min.y < max.y;
    }

    constexpr auto contains(const MyPoint& p) const -> bool {
        return min.x <= p.x && p.x < max.x && min.y <= p.y && p.y < max.y;
    }
};

TEST_CASE("Generic functions") {
    MyPoint p1{1, 2};
    MyPoint p2{1, 2};
    MyPoint p3{2, 3};

    CHECK(recti::overlap(p1, p2));
    CHECK(!recti::overlap(p1, p3));

    MyRect r1{{0, 0}, {4, 4}};
    MyRect r2{{2, 2}, {6, 6}};
    MyRect r3{{8, 8}, {10, 10}};

    CHECK(recti::overlap(r1, r2));
    CHECK(!recti::overlap(r1, r3));

    CHECK(recti::contain(r1, p1));
    CHECK(recti::contain(r1, p3));
}
