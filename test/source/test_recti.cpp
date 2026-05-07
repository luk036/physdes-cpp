#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

// #include <recti/halton_int.hpp>  // for recti
#include <recti/recti.hpp>  // for Rectangle, HSegment, VSegment
// #include <random>
#include <ostream>  // for doctest

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

    CHECK_FALSE(a == b);
    CHECK(a != b);
    CHECK_EQ((a + v) + v, b);
    CHECK_EQ((a + v) - v, a);
    CHECK_EQ(a.flip_xy().flip_xy(), a);
    CHECK_EQ(a.flip_y().flip_y(), a);
}

TEST_CASE("Point test hull") {
    const auto a = Point<int>{3, 5};
    const auto b = Point<int>{5, 7};
    CHECK_EQ(a.hull_with(b), Point<Interval<int>>({3, 5}, {5, 7}));
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

    CHECK_NE(r1, r2);
    CHECK_EQ(r3 + v, r1);

    CHECK(r1.contains(p));
    CHECK(r1.contains(r2));
    CHECK(r1.overlaps(r2));
    CHECK(overlap(r1, r2));

    CHECK_EQ(r1.min_dist_with(r2), 0);
    CHECK_EQ(min_dist(r1, r2), 0);
}

TEST_CASE("Segment test") {
    const auto xrng1 = Interval<int>{4000, 8000};
    const auto yrng1 = Interval<int>{5000, 7000};
    const auto s1 = HSegment<int>{xrng1, 6000};
    const auto s2 = VSegment<int>{5000, yrng1};

    CHECK(s1.overlaps(s2));
}

TEST_CASE("Detect overlap basic") {
    const auto r1 = Rectangle<int>{Interval{0, 5}, Interval{0, 5}};
    const auto r2 = Rectangle<int>{Interval{3, 8}, Interval{3, 8}};
    auto result = detect_overlap(std::vector<Rectangle<int>>{r1, r2});
    CHECK(result.has_value());
}

TEST_CASE("Detect overlap no overlap") {
    const auto r1 = Rectangle<int>{Interval{0, 2}, Interval{0, 2}};
    const auto r2 = Rectangle<int>{Interval{3, 5}, Interval{3, 5}};
    auto result = detect_overlap(std::vector<Rectangle<int>>{r1, r2});
    CHECK_FALSE(result.has_value());
}

TEST_CASE("Detect overlap multiple rectangles") {
    const auto r1 = Rectangle<int>{Interval{0, 2}, Interval{0, 2}};
    const auto r2 = Rectangle<int>{Interval{1, 3}, Interval{1, 3}};
    const auto r3 = Rectangle<int>{Interval{10, 12}, Interval{10, 12}};
    auto result = detect_overlap(std::vector<Rectangle<int>>{r1, r2, r3});
    CHECK(result.has_value());
}

TEST_CASE("Detect overlap single rectangle") {
    const auto r1 = Rectangle<int>{Interval{0, 5}, Interval{0, 5}};
    auto result = detect_overlap(std::vector<Rectangle<int>>{r1});
    CHECK_FALSE(result.has_value());
}

TEST_CASE("Detect overlap empty list") {
    auto result = detect_overlap(std::vector<Rectangle<int>>{});
    CHECK_FALSE(result.has_value());
}

TEST_CASE("Detect overlap touching edges") {
    const auto r1 = Rectangle<int>{Interval{0, 5}, Interval{0, 5}};
    const auto r2 = Rectangle<int>{Interval{5, 10}, Interval{5, 10}};
    auto result = detect_overlap(std::vector<Rectangle<int>>{r1, r2});
    CHECK_FALSE(result.has_value());
}

TEST_CASE("Detect overlap partial y overlap") {
    const auto r1 = Rectangle<int>{Interval{0, 5}, Interval{0, 3}};
    const auto r2 = Rectangle<int>{Interval{3, 8}, Interval{2, 6}};
    auto result = detect_overlap(std::vector<Rectangle<int>>{r1, r2});
    CHECK(result.has_value());
}

TEST_CASE("Detect overlap no x overlap") {
    const auto r1 = Rectangle<int>{Interval{0, 2}, Interval{0, 5}};
    const auto r2 = Rectangle<int>{Interval{3, 5}, Interval{0, 5}};
    auto result = detect_overlap(std::vector<Rectangle<int>>{r1, r2});
    CHECK_FALSE(result.has_value());
}

TEST_CASE("Detect overlap 11 rectangles") {
    std::vector<Rectangle<int>> rects = {
        Rectangle<int>{Interval{0, 4}, Interval{0, 4}},
        Rectangle<int>{Interval{2, 6}, Interval{2, 6}},
        Rectangle<int>{Interval{5, 9}, Interval{5, 9}},
        Rectangle<int>{Interval{8, 12}, Interval{8, 12}},
        Rectangle<int>{Interval{11, 15}, Interval{11, 15}},
        Rectangle<int>{Interval{3, 7}, Interval{10, 14}},
        Rectangle<int>{Interval{14, 18}, Interval{14, 18}},
        Rectangle<int>{Interval{16, 20}, Interval{4, 8}},
        Rectangle<int>{Interval{6, 10}, Interval{6, 10}},
        Rectangle<int>{Interval{9, 13}, Interval{12, 16}},
        Rectangle<int>{Interval{0, 3}, Interval{8, 11}},
    };
    auto result = detect_overlap(rects);
    CHECK(result.has_value());
}
