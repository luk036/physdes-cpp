#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST...

#include <recti/halton_int.hpp>  // for recti
#include <recti/merge_obj.hpp>   // for MergeObj, operator+, operator-
// #include <random>
#include <iostream>  // for operator<<, ostream, basic_ostream

#include "recti/interval.hpp"  // for min_dist, overlap, interval
#include "recti/point.hpp"     // for operator<<, Point
#include "recti/vector2.hpp"   // for vector2

// using std::randint;
using namespace recti;

TEST_CASE("MergeObj test") {
    auto r1 = MergeObj{4 + 5, 4 - 5};
    auto r2 = MergeObj{7 + 9, 7 - 9};
    auto v = Vector2{5, 6};

    CHECK(r1 != r2);
    CHECK((r1 - v) + v == r1);
    CHECK(!overlap(r1, r2));
    CHECK(r1.min_dist_with(r2) == 7);
    CHECK(min_dist(r1, r2) == 7);
}

/*
x [ - a ,  + a ]
y [ - a ,  + a ]

x [ + a ,  - a ]
y [ + a ,  - a ]

*/
TEST_CASE("merge test") {
    auto s1 = MergeObj{800, -400};
    auto s2 = MergeObj{1400, -400};
    auto m1 = s1.merge_with(s2);
    std::cout << m1 << '\n';
    // CHECK(false);
    CHECK(m1 == MergeObj(interval{1100, 1100}, interval{-700, -100}));
}
