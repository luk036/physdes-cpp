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

TEST_CASE("Point 3D test") {
    auto a = point{point{40000, 80000}, 20000};
    auto b = point{point{50000, 60000}, 10000};
    auto v = (b - a) / 2;  // integer division

    CHECK(a < b);
    CHECK(a <= b);
    CHECK(!(a == b));
    CHECK(a != b);
    CHECK(b > a);
    CHECK(b >= a);
    CHECK((a + v) + v == b);  // may not true due to integer division
    CHECK((a - v) + v == a);

    CHECK(a.flip_xy().flip_xy() == a);
    // CHECK(a.flip_y().flip_y() == a);
}

TEST_CASE("Interval test") {
    auto a = point{interval{4, 8}, 1};
    auto b = point{interval{5, 6}, 1};
    auto v = vector2{3, 0};

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

    CHECK(a.contains(b));
    CHECK(a.intersection_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));

    CHECK(min_dist(a, b) == 0);
}

TEST_CASE("Rectangle 3D test") {
    auto xrng1 = interval{40000, 80000};
    auto yrng1 = interval{50000, 70000};
    auto r1 = point{rectangle{xrng1, yrng1}, 1000};
    auto xrng2 = interval{50000, 70000};
    auto yrng2 = interval{60000, 60000};
    auto r2 = point{rectangle{xrng2, yrng2}, 1000};
    auto v = vector2{vector2{50000, 60000}, 0};
    auto p1 = point{point{70000, 60000}, 1000};
    auto p2 = point{point{70000, 60000}, 2000};

    CHECK(r1 != r2);
    CHECK((r1 - v) + v == r1);

    // CHECK(r1 <= p);
    CHECK(r1.contains(p1));
    CHECK(!r1.contains(p2));
    CHECK(r1.contains(r2));
    CHECK(r1.overlaps(r2));
    CHECK(overlap(r1, r2));

    CHECK(r1.min_dist_with(r2) == 0);
    CHECK(min_dist(r1, r2) == 0);

    CHECK(r1.min_dist_with(p2) == p2.min_dist_with(r1));
    // CHECK(min_dist(r1, p2) == min_dist(p2, r1));
}

// TEST_CASE("Segment test") {
//     auto xrng1 = interval{4, 8};
//     auto yrng1 = interval{5, 7};
//     auto s1 = hsegment{xrng1, 6};
//     auto s2 = vsegment{5, yrng1};

//     CHECK(s1.overlaps(s2));
// }

// TEST_CASE("Interval overlapping test") {
//     constexpr auto N = 20;
//     auto lst = std::list<interval<unsigned int>>{};
//     auto hgenX = vdcorput(3, 7);
//     // auto hgenY = vdcorput(2, 11);

//     for (auto i = 0; i != N; ++i) {
//         for (auto j = 0; j != N; ++j) {
//             auto x = hgenX();
//             // auto y = hgenY();
//             auto xrng = interval{x, x + 100};
//             // auto yrng = interval{y, y + 100};
//             // auto r = rectangle{xrng, yrng};
//             // lst.push_back(r);
//             lst.push_back(xrng);
//         }
//     }

//     std::set<interval<unsigned int>> S;   // set of maximal non-overlapped rectangles
//     std::list<interval<unsigned int>> L;  // list of the removed rectangles

//     for (const auto& intvl : lst) {
//         if (S.find(intvl) != S.end()) {
//             L.push_back(intvl);
//         } else {
//             S.insert(intvl);
//         }
//     }

//     // for (const auto& r : S)
//     // {
//     //     cout << "  \\draw " << r << ";\n";
//     // }
//     // for (const auto& r : L)
//     // {
//     //     cout << "  \\draw[color=red] " << r << ";\n";
//     // }
// }

TEST_CASE("merge_obj test") {
    auto a = point{40000, 80000};
    auto b = point{50000, 60000};

    auto m1 = merge_obj{a + 50000, a - 50000};
    auto m2 = merge_obj{b + 90000, b - 90000};
    // auto v = vector2{vector2{50000, 60000}, 0};

    CHECK(m1 != m2);
    // CHECK((m1 - v) + v == m1);
    CHECK(!overlap(m1, m2));
    CHECK(m1.min_dist_with(m2) == m2.min_dist_with(m1));
    CHECK(min_dist(m1, m2) == min_dist(m2, m1));
}
