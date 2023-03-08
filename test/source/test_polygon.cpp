#include <doctest/doctest.h> // for ResultBuilder, CHECK, TestCase, Expr...
// #include <fmt/core.h>
#include <gsl/span>             // for span
#include <recti/halton_int.hpp> // for Vdcorput, recti
#include <recti/polygon.hpp>    // for Polygon, polygon_is_clockwise, creat...
#include <vector>               // for vector

#include "recti/point.hpp" // for Point

// using std::randint;
using namespace recti;

TEST_CASE("Polygon test (ycoord-mono)") {
  auto S = std::vector<Point<int>>{{-2, 2}, {0, -1}, {-5, 1},  {-2, 4},
                                   {0, -4}, {-4, 3}, {-6, -2}, {5, 1},
                                   {2, 2},  {3, -3}, {-3, -4}, {1, 4}};

  create_ymono_polygon(S.begin(), S.end());
  auto P = Polygon<int>(S);
  CHECK(P.signed_area_x2() == 102);
  CHECK(!polygon_is_clockwise<int>(S));
  CHECK(!point_in_polygon<int>(S, Point{4, 5}));
}

// 106980095362

TEST_CASE("Polygon test (xcoord-mono)") {
  auto S = std::vector<Point<int>>{{-2, 2}, {0, -1}, {-5, 1},  {-2, 4},
                                   {0, -4}, {-4, 3}, {-6, -2}, {5, 1},
                                   {2, 2},  {3, -3}, {-3, -4}, {1, 4}};
  create_xmono_polygon(S.begin(), S.end());
  auto P = Polygon<int>(S);
  CHECK(P.signed_area_x2() == 110);
  CHECK(!polygon_is_clockwise<int>(S));
}

TEST_CASE("Polygon test (ycoord-mono 50)") {
  auto hgenX = Vdcorput(3, 7);
  auto hgenY = Vdcorput(2, 11);
  auto S = std::vector<Point<int>>{};
  for (auto i = 0U; i != 50; ++i) {
    S.emplace_back(Point<int>(int(hgenX()), int(hgenY())));
  }
  create_ymono_polygon(S.begin(), S.end());
  // fmt::print(
  //     "\n<svg viewBox='0 0 2187 2048'
  //     xmlns='http://www.w3.org/2000/svg'>\n");
  // fmt::print("  <polygon points='");
  // for (auto&& p : S)
  // {
  //     fmt::print("{},{} ", p.xcoord(), p.ycoord());
  // }
  // fmt::print("'\n");
  // fmt::print("  fill='#88C0D0' stroke='black' />\n");
  // for (auto&& p : S)
  // {
  //     fmt::print("  <circle cx='{}' cy='{}' r='10' />\n", p.xcoord(),
  //     p.ycoord());
  // }

  auto q = Point<int>(int(hgenX()), int(hgenY()));
  // fmt::print(
  //     "  <circle cx='{}' cy='{}' r='10' fill='#BF616A' />\n", q.xcoord(),
  //     q.ycoord());
  // fmt::print("</svg>\n");

  auto P = Polygon<int>(S);
  CHECK(P.signed_area_x2() == 4409856);
  CHECK(!polygon_is_clockwise<int>(S));
  CHECK(point_in_polygon<int>(S, q));
}
