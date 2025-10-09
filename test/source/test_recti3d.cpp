#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <ostream>               // for operator<<
#include <recti/halton_int.hpp>  // for recti
#include <recti/merge_obj.hpp>   // for merge_obj
#include <recti/recti.hpp>       // for Rectangle

#include "recti/interval.hpp"  // for min_dist, Interval, overlap
#include "recti/point.hpp"     // for Point, operator<<, operator+, operator-
#include "recti/vector2.hpp"   // for Vector2, operator/

// using std::randint;
using namespace recti;

// template <typename T> struct my_point : Point<T, T> { double data; };

TEST_CASE("Point 3D test") {
    const auto a = Point<Point<int>, int>{Point<int>{40000, 80000}, 20000};
    const auto b = Point<Point<int>, int>{Point<int>{50000, 60000}, 10000};
    const auto v = (b - a) / 2;  // integer division

    CHECK(!(a == b));
    CHECK(a != b);
    CHECK((a + v) + v == b);  // may not true due to integer division
    CHECK((a + v) - v == a);
    CHECK(a.flip_xy().flip_xy() == a);
    // CHECK(a.flip_y().flip_y() == a);
}

TEST_CASE("Interval test") {
    const auto a = Point<Interval<int>, int>{Interval<int>{4, 8}, 1};
    const auto b = Point<Interval<int>, int>{Interval<int>{5, 6}, 1};
    // auto v = Vector2<int>{3, 0};

    CHECK(!(b == a));
    CHECK(b != a);
    CHECK(a.contains(b));
    CHECK(a.intersect_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));

    CHECK(min_dist(a, b) == 0);
}

TEST_CASE("Rectangle 3D test") {
    const auto xrng1 = Interval<int>{40000, 80000};
    const auto yrng1 = Interval<int>{50000, 70000};
    const auto r1 = Point<Rectangle<int>, int>{Rectangle<int>{xrng1, yrng1}, 1000};
    const auto xrng2 = Interval<int>{50000, 70000};
    const auto yrng2 = Interval<int>{60000, 60000};
    const auto r2 = Point<Rectangle<int>, int>{Rectangle<int>{xrng2, yrng2}, 1000};
    // auto v = Vector2<Vector2<int>, int>{Vector2<int>{50000, 60000}, 0};
    const auto p1 = Point<Point<int>, int>{Point<int>{70000, 60000}, 1000};
    const auto p2 = Point<Point<int>, int>{Point<int>{70000, 60000}, 2000};

    CHECK(r1 != r2);
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
//     auto xrng1 = Interval{4, 8};
//     auto yrng1 = Interval{5, 7};
//     auto s1 = HSegment{xrng1, 6};
//     auto s2 = VSegment{5, yrng1};

//     CHECK(s1.overlaps(s2));
// }

// TEST_CASE("Interval overlapping test") {
//     constexpr auto N = 20;
//     auto lst = std::list<Interval<unsigned int>>{};
//     auto hgenX = Vdcorput(3, 7);
//     // auto hgenY = Vdcorput(2, 11);

//     for (auto i = 0; i != N; ++i) {
//         for (auto j = 0; j != N; ++j) {
//             auto xcoord = hgenX();
//             // auto ycoord = hgenY();
//             auto xrng = Interval{xcoord, xcoord + 100};
//             // auto yrng = Interval{ycoord, ycoord + 100};
//             // auto r = Rectangle{xrng, yrng};
//             // lst.push_back(r);
//             lst.push_back(xrng);
//         }
//     }

//     std::set<Interval<unsigned int>> S;   // set of maximal non-overlapped
//     rectangles std::list<Interval<unsigned int>> L;  // list of the removed
//     rectangles

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

// TEST_CASE("MergeObj test") {
//     auto a = Point<int>{40000, 80000};
//     auto b = Point<int>{50000, 60000};

//     auto m1 = MergeObj<Point<int>>{a + 50000, a - 50000};
//     auto m2 = MergeObj<Point<int>>{b + 90000, b - 90000};
//     // auto v = Vector2{Vector2{50000, 60000}, 0};

//     CHECK(m1 != m2);
//     // CHECK((m1 - v) + v == m1);
//     CHECK(!overlap(m1, m2));
//     CHECK(m1.min_dist_with(m2) == m2.min_dist_with(m1));
//     CHECK(min_dist(m1, m2) == min_dist(m2, m1));
// }
