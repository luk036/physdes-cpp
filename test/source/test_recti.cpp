#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <recti/halton_int.hpp>  // for recti
#include <recti/recti.hpp>       // for Rectangle, HSegment, VSegment
// #include <random>
#include <ostream>  // for operator<<

#include "recti/interval.hpp"  // for Interval, min_dist, overlap
#include "recti/point.hpp"     // for Point, operator<<, operator+, operat...
#include "recti/vector2.hpp"   // for operator/, vector2

// using std::randint;
using namespace recti;

// template <typename T> struct my_point : Point<T, T> { double data; };

TEST_CASE("Point test") {
    const auto a = Point<int>{4000, 8000};
    const auto b = Point<int>{5000, 6000};
    const auto v = (b - a) / 2;  // integer division

    CHECK(!(a == b));
    CHECK(a != b);
    CHECK((a + v) + v == b);
    CHECK((a + v) - v == a);
    CHECK(a.flip_xy().flip_xy() == a);
    CHECK(a.flip_y().flip_y() == a);
}

TEST_CASE("Point test hull") {
    const auto a = Point<int>{3, 5};
    const auto b = Point<int>{5, 7};
    CHECK(a.hull_with(b) == Point<Interval<int>>({3, 5}, {5, 7}));
}

TEST_CASE("Rectangle test") {
    const auto xrng1 = Interval<int>{4000, 8000};
    const auto yrng1 = Interval<int>{5000, 7000};
    const auto r1 = Rectangle<int>{xrng1, yrng1};
    const auto xrng2 = Interval<int>{5000, 7000};
    const auto yrng2 = Interval<int>{6000, 6000};
    const auto r2 = Rectangle<int>{xrng2, yrng2};
    const auto p = Point<int>{7000, 6000};
    const auto v = Vector2<int>{5000, 6000};
    const auto r3 = Rectangle<int>(r1 - v);

    CHECK(r1 != r2);
    CHECK(r3 + v == r1);

    CHECK(r1.contains(p));
    CHECK(r1.contains(r2));
    CHECK(r1.overlaps(r2));
    CHECK(overlap(r1, r2));

    CHECK(r1.min_dist_with(r2) == 0);
    CHECK(min_dist(r1, r2) == 0);
}

TEST_CASE("Segment test") {
    const auto xrng1 = Interval<int>{4000, 8000};
    const auto yrng1 = Interval<int>{5000, 7000};
    const auto s1 = HSegment<int>{xrng1, 6000};
    const auto s2 = VSegment<int>{5000, yrng1};

    CHECK(s1.overlaps(s2));
}
