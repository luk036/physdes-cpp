#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <list>                  // for list, __list_iterator, operator!=
#include <ostream>               // for operator<<
#include <recti/halton_int.hpp>  // for Vdcorput, recti
#include <recti/interval.hpp>    // for Interval, operator<<, operator+, ope...
#include <set>                   // for set, set<>::iterator

// using std::randint;
using namespace recti;

// template <typename T> struct my_point : Point<T, T> { double data; };

TEST_CASE("Interval test") {
    auto a = Interval{4, 8};
    auto b = Interval{5, 6};
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
    CHECK(a.intersection_with(8) == Interval{8, 8});
    CHECK(a.contains(b));
    CHECK(a.intersection_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));

    CHECK(min_dist(a, b) == 0);
}

TEST_CASE("Interval of Interval test") {
    auto a = Interval{Interval{3, 4}, Interval{8, 9}};
    auto b = Interval{Interval{5, 6}, Interval{6, 7}};
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

    CHECK(a.contains(Interval{4, 5}));
    CHECK(a.contains(Interval{7, 8}));

    CHECK(a.intersection_with(Interval{7, 8}) == Interval{Interval{7, 7}, Interval{8, 8}});  // ???

    CHECK(a.contains(b));
    CHECK(a.intersection_with(b) == b);
    CHECK(!b.contains(a));
    CHECK(a.overlaps(b));
    CHECK(b.overlaps(a));

    // CHECK(min_dist(a, b) == 0);
}

TEST_CASE("Interval overlapping test") {
    constexpr auto N = 20;
    auto lst = std::list<Interval<unsigned int>>{};
    auto hgenX = Vdcorput(3, 7);
    // auto hgenY = Vdcorput(2, 11);

    for (auto i = 0; i != N; ++i) {
        for (auto j = 0; j != N; ++j) {
            auto x = hgenX();
            // auto y = hgenY();
            auto xrng = Interval{x, x + 100};
            // auto yrng = Interval{y, y + 100};
            // auto r = Rectangle{xrng, yrng};
            // lst.push_back(r);
            lst.push_back(xrng);
        }
    }

    std::set<Interval<unsigned int>> S;   // set of maximal non-overlapped rectangles
    std::list<Interval<unsigned int>> L;  // list of the removed rectangles

    for (const auto &intvl : lst) {
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
