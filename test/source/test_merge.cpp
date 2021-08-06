#include <doctest/doctest.h>
// #include <random>
#include <iostream>
#include <recti/halton_int.hpp>
#include <recti/merge_obj.hpp>
#include <recti/recti.hpp>

// using std::randint;
using namespace recti;


TEST_CASE("merge_obj test") {
    auto r1 = merge_obj{4 + 5, 4 - 5};
    auto r2 = merge_obj{7 + 9, 7 - 9};
    auto v = vector2{5, 6};

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
    auto s1 = merge_obj{800, -400};
    auto s2 = merge_obj{1400, -400};
    auto m1 = s1.merge_with(s2);
    std::cout << m1 << '\n';
    // CHECK(false);
    CHECK(m1 == merge_obj(interval{1100, 1100}, interval{-700, -100}));
}
