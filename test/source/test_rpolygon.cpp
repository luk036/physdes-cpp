#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs
#include <fmt/core.h>

#include <gsl/span>            // for span
#include <ldsgen/ilds.hpp>     // for VdCorput
#include <recti/rpolygon.hpp>  // for RPolygon, RPolygon_is_clockwise, cre...
#include <vector>              // for vector

#include "recti/point.hpp"  // for Point, operator>

using namespace recti;

TEST_CASE("Rectilinear Polygon test (ycoord-mono)") {
    auto S = std::vector<Point<int>>{{-2, 2},  {0, -1}, {-5, 1}, {-2, 4}, {0, -4},  {-4, 3},
                                     {-6, -2}, {5, 1},  {2, 2},  {3, -3}, {-3, -4}, {1, 4}};
    auto is_clockwise = create_ymono_rpolygon(S.begin(), S.end());
    auto P = RPolygon<int>(S);
    CHECK(!is_clockwise);
    CHECK_EQ(P.signed_area(), 45);
    CHECK(!rpolygon_is_clockwise<int>(S));
    CHECK(!point_in_rpolygon<int>(S, Point<int>{4, 5}));
}

TEST_CASE("Rectilinear Polygon test (xcoord-mono)") {
    auto S = std::vector<Point<int>>{{-2, 2},  {0, -1}, {-5, 1}, {-2, 4}, {0, -4},  {-4, 3},
                                     {-6, -2}, {5, 1},  {2, 2},  {3, -3}, {-3, -4}, {1, 4}};
    auto is_anticlockwise = create_xmono_rpolygon(S.begin(), S.end());
    auto P = RPolygon<int>(S);
    CHECK(!is_anticlockwise);
    CHECK_EQ(P.signed_area(), -53);
    CHECK(rpolygon_is_clockwise<int>(S));
}

TEST_CASE("Rectilinear Polygon test (ycoord-mono 50)") {
    auto hgenX = ildsgen::VdCorput(3, 7);
    auto hgenY = ildsgen::VdCorput(2, 11);
    auto S = std::vector<Point<int>>{};
    for (auto i = 0; i != 50; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    auto is_clockwise = create_ymono_rpolygon(S.begin(), S.end());
    auto q = Point<int>(int(hgenX.pop()), int(hgenY.pop()));

    // fmt::print(
    //     "\n<svg viewBox='0 0 2187 2048'
    //     xmlns='http://www.w3.org/2000/svg'>\n");
    // fmt::print("  <polygon points='");
    // auto p0 = S.back();
    // for (auto&& p1 : S)
    // {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(),
    //     p0.ycoord()); p0 = p1;
    // }
    // fmt::print("'\n");
    // fmt::print("  fill='#88C0D0' stroke='black' />\n");
    // for (auto&& p : S)
    // {
    //     fmt::print("  <circle cx='{}' cy='{}' r='10' />\n", p.xcoord(),
    //     p.ycoord());
    // }
    // fmt::print(
    //     "  <circle cx='{}' cy='{}' r='10' fill='#BF616A' />\n", q.xcoord(),
    //     q.ycoord());
    // fmt::print("</svg>\n");

    auto P = RPolygon<int>(S);
    CHECK(is_clockwise);
    CHECK_EQ(P.signed_area(), -2032128);
    CHECK(rpolygon_is_clockwise<int>(S));
    CHECK(!point_in_rpolygon<int>(S, q));
}
