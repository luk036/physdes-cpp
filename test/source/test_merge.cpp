#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST...

#include <iostream>              // for operator<<, ostream, basic_ostream
#include <recti/halton_int.hpp>  // for recti
#include <recti/merge_obj.hpp>   // for MergeObj, operator+, operator-

#include "recti/interval.hpp"  // for min_dist, overlap, interval

using namespace recti;

TEST_CASE("MergeObj test") {
    const auto r1 = MergeObj<int>{4 - 5, 4 + 5};
    const auto r2 = MergeObj<int>{7 - 9, 7 + 9};

    CHECK_EQ(r1, r1);
    CHECK(r1 != r2);
    CHECK(!overlap(r1, r2));
    CHECK(r1.min_dist_with(r2) == 7);
    CHECK(min_dist(r1, r2) == 7);
}

TEST_CASE("merge test") {
    const auto s1 = MergeObj<int>{800, -400};
    const auto s2 = MergeObj<int>{1400, -400};
    const auto m1 = s1.merge_with(s2);
    std::cout << m1 << '\n';
    // CHECK(false);
    CHECK(m1 == MergeObj<Interval<int>>(Interval<int>{1100, 1100}, Interval<int>{-700, -100}));
}
