#include <doctest/doctest.h>
// #include <fmt/core.h>
#include <recti/halton_int.hpp>
#include <recti/polygon.hpp>
#include <recti/recti.hpp>
#include <vector>

// using std::randint;
using namespace recti;

TEST_CASE("Polygon test (y-mono)") {
    auto S = std::vector<point<int>>{{-2, 2},  {0, -1}, {-5, 1}, {-2, 4}, {0, -4},  {-4, 3},
                                     {-6, -2}, {5, 1},  {2, 2},  {3, -3}, {-3, -4}, {1, 4}};

    create_ymono_polygon(S.begin(), S.end());
    auto P = polygon<int>(S);
    CHECK(P.signed_area_x2() == 102);
    CHECK(!point_in_polygon<int>(S, point{4, 5}));
}

// 106980095362

TEST_CASE("Polygon test (x-mono)") {
    auto S = std::vector<point<int>>{{-2, 2},  {0, -1}, {-5, 1}, {-2, 4}, {0, -4},  {-4, 3},
                                     {-6, -2}, {5, 1},  {2, 2},  {3, -3}, {-3, -4}, {1, 4}};
    create_xmono_polygon(S.begin(), S.end());
    auto P = polygon<int>(S);
    CHECK(P.signed_area_x2() == 110);
}

TEST_CASE("Polygon test (y-mono 50)") {
    auto hgenX = vdcorput(3, 7);
    auto hgenY = vdcorput(2, 11);
    auto S = std::vector<point<int>>{};
    for (auto i = 0U; i != 50; ++i) {
        S.emplace_back(point<int>(int(hgenX()), int(hgenY())));
    }
    create_ymono_polygon(S.begin(), S.end());
    // fmt::print(
    //     "\n<svg viewBox='0 0 2187 2048' xmlns='http://www.w3.org/2000/svg'>\n");
    // fmt::print("  <polygon points='");
    // for (auto&& p : S)
    // {
    //     fmt::print("{},{} ", p.x(), p.y());
    // }
    // fmt::print("'\n");
    // fmt::print("  fill='#88C0D0' stroke='black' />\n");
    // for (auto&& p : S)
    // {
    //     fmt::print("  <circle cx='{}' cy='{}' r='10' />\n", p.x(), p.y());
    // }

    auto q = point<int>(int(hgenX()), int(hgenY()));
    // fmt::print(
    //     "  <circle cx='{}' cy='{}' r='10' fill='#BF616A' />\n", q.x(), q.y());
    // fmt::print("</svg>\n");

    auto P = polygon<int>(S);
    CHECK(P.signed_area_x2() == 4409856);
    CHECK(point_in_polygon<int>(S, q));
}
