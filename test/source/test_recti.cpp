#include <doctest/doctest.h>
// #include <random>
// #include <iostream>
#include <list>
#include <recti/recti.hpp>
#include <set>

// using std::randint;
using namespace recti;

// template <typename T> struct my_point : point<T, T> { double data; };

TEST_CASE("Point test") {
    auto a = point<int>{4, 8};
    auto b = point<int>{5, 6};

    CHECK(a < b);
    CHECK(a <= b);
    CHECK(!(a == b));
    CHECK(a != b);
    CHECK(b > a);
    CHECK(b >= a);
}

TEST_CASE("Interval test") {
    auto a = interval{4, 8};
    auto b = interval{5, 6};

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

    CHECK(a.contains(4));
    CHECK(a.contains(8));
    CHECK(a.contains(b));
    CHECK(!b.contains(a));
}

TEST_CASE("Rectangle test") {
    auto xrng1 = interval{4, 8};
    auto yrng1 = interval{5, 7};
    auto r1 = rectangle{xrng1, yrng1};
    // auto xrng2 = interval {5, 7};
    // auto yrng2 = interval {6, 6};
    // auto r2 = rectangle {xrng2, yrng2};
    auto p = point{7, 6};

    CHECK(r1.contains(p));
    // CHECK(r1.contains(r2));
}

TEST_CASE("Rectilinear test") {
    constexpr auto N = 20;
    auto lst = std::list<rectangle<int>>{};

    for (auto i = 0; i != N; ++i) {
        int ii = i * 100;
        for (auto j = 0; j != N; ++j) {
            int jj = j * 100;
            // auto xrng = interval {ii, ii + randint(50, 110)};
            // auto yrng = interval {jj, jj + randint(50, 110)};
            auto xrng = interval{ii, ii + std::rand() % 100};
            auto yrng = interval{jj, jj + std::rand() % 100};
            auto r = rectangle{xrng, yrng};
            lst.push_back(r);
        }
    }

    std::set<rectangle<int>> S;   // set of maximal non-overlapped rectangles
    std::list<rectangle<int>> L;  // list of the removed rectangles

    for (const auto& r : lst) {
        auto search = S.find(r);
        if (search != S.end()) {
            L.push_back(r);
        } else {
            S.insert(r);
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
