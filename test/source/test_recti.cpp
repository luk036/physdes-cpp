#include <doctest/doctest.h>
// #include <random>
#include <cstdlib>
#include <list>
#include <ostream>
#include <recti/halton_int.hpp>
#include <recti/merge_obj.hpp>
#include <recti/recti.hpp>
#include <set>

// using std::randint;
using namespace recti;

// template <typename T> struct my_point : point<T, T> { double data; };

TEST_CASE("Point test") {
    auto a = point<int>{4, 8};
    auto b = point<int>{5, 6};
    auto v = (b - a) / 2;  // integer division

    CHECK(a < b);
    CHECK(a <= b);
    CHECK(!(a == b));
    CHECK(a != b);
    CHECK(b > a);
    CHECK(b >= a);
    CHECK((a + v) + v != b);  // due to integer division
    CHECK((a - v) + v == a);

    CHECK(a.flip_xy().flip_xy() == a);
    CHECK(a.flip_y().flip_y() == a);
}

TEST_CASE("Interval test") {
    auto a = interval{4, 8};
    auto b = interval{5, 6};
    auto v = 3;

    CHECK(!(a < b));
    CHECK(!(b < a));
    CHECK(!(a > b));
    CHECK(!(b > a));
    CHECK(a <= b);
    CHECK(b <= a);
    CHECK(a >= b);
    CHECK(b >= a);

    CHECK(!(b == a));
    CHECK(b != a);

    CHECK((a - v) + v == a);

    CHECK(a.contains(4));
    CHECK(a.contains(8));
    CHECK(a.intersection_with(8) == interval{8, 8});
    CHECK(a.contains(b));
    CHECK(a.intersection_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));

    CHECK(min_dist(a, b) == 0);
}

TEST_CASE("Rectangle test") {
    auto xrng1 = interval{4, 8};
    auto yrng1 = interval{5, 7};
    auto r1 = rectangle{xrng1, yrng1};
    auto xrng2 = interval{5, 7};
    auto yrng2 = interval{6, 6};
    auto r2 = rectangle{xrng2, yrng2};
    auto p = point{7, 6};
    auto v = vector2{5, 6};

    CHECK(r1 != r2);
    CHECK((r1 - v) + v == r1);

    CHECK(r1 <= p);
    CHECK(r1.contains(p));
    CHECK(r1.contains(r2));
    CHECK(r1.overlaps(r2));
    CHECK(overlap(r1, r2));

    CHECK(r1.min_dist_with(r2) == 0);
    CHECK(min_dist(r1, r2) == 0);
}

TEST_CASE("Segment test") {
    auto xrng1 = interval{4, 8};
    auto yrng1 = interval{5, 7};
    auto s1 = hsegment{xrng1, 6};
    auto s2 = vsegment{5, yrng1};

    CHECK(s1.overlaps(s2));
}

TEST_CASE("Interval overlapping test") {
    constexpr auto N = 20;
    auto lst = std::list<interval<unsigned int>>{};
    auto hgenX = vdcorput(3, 7);
    // auto hgenY = vdcorput(2, 11);

    for (auto i = 0; i != N; ++i) {
        for (auto j = 0; j != N; ++j) {
            auto x = hgenX();
            // auto y = hgenY();
            auto xrng = interval{x, x + 100};
            // auto yrng = interval{y, y + 100};
            // auto r = rectangle{xrng, yrng};
            // lst.push_back(r);
            lst.push_back(xrng);
        }
    }

    std::set<interval<unsigned int>> S;   // set of maximal non-overlapped rectangles
    std::list<interval<unsigned int>> L;  // list of the removed rectangles

    for (const auto& intvl : lst) {
        if (S.find(intvl) != S.end()) {
            L.push_back(intvl);
        } else {
            S.insert(intvl);
        }
    }

    // for (const auto& r : S)
    // {
    //     cout << "  \\draw " << r << ";\n";
    // }
    // for (const auto& r : L)
    // {
    //     cout << "  \\draw[color=red] " << r << ";\n";
    // }
}

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
