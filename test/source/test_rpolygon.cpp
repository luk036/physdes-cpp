#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs
#include <fmt/core.h>

#include <ostream>

// #include <span>            // for span
#include <ldsgen/ilds.hpp>     // for VdCorput
#include <recti/rpolygon.hpp>  // for RPolygon, RPolygon_is_clockwise, cre...
#include <vector>              // for vector

#include "recti/point.hpp"  // for Point, operator>

using namespace recti;

TEST_CASE("Rectilinear Polygon test (ycoord-mono)") {
    auto S = std::vector<Point<int>>{{-2, 2},  {0, -1}, {-5, 1}, {-2, 4}, {0, -4},  {-4, 3},
                                     {-6, -2}, {5, 1},  {2, 2},  {3, -3}, {-3, -4}, {1, 4}};
    const auto is_clockwise = create_ymono_rpolygon(S.begin(), S.end());
    CHECK(!is_clockwise);
    CHECK(rpolygon_is_anticlockwise<int>(S));
    CHECK(rpolygon_is_ymonotone<int>(S));
    CHECK(!rpolygon_is_xmonotone<int>(S));
    CHECK(!point_in_rpolygon<int>(S, Point<int>{4, 5}));
    const auto P = RPolygon<int>(S);
    CHECK_EQ(P.signed_area(), 45);
}

TEST_CASE("Rectilinear Polygon test (xcoord-mono)") {
    auto S = std::vector<Point<int>>{{-2, 2},  {0, -1}, {-5, 1}, {-2, 4}, {0, -4},  {-4, 3},
                                     {-6, -2}, {5, 1},  {2, 2},  {3, -3}, {-3, -4}, {1, 4}};
    const auto is_anticlockwise = create_xmono_rpolygon(S.begin(), S.end());

    CHECK(is_anticlockwise);
    CHECK(rpolygon_is_anticlockwise<int>(S));
    CHECK(rpolygon_is_xmonotone<int>(S));
    CHECK(!rpolygon_is_ymonotone<int>(S));
    const auto P = RPolygon<int>(S);
    CHECK_EQ(P.signed_area(), 51);
}

TEST_CASE("Rectilinear Polygon test (xcoord-mono 50)") {
    auto hgenX = ildsgen::VdCorput(3, 7);
    auto hgenY = ildsgen::VdCorput(2, 11);
    auto S = std::vector<Point<int>>{};
    for (auto i = 0; i != 50; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    const auto is_anticlockwise = create_xmono_rpolygon(S.begin(), S.end());

    // SVG output (commented out as per reference)
    // fmt::print("<svg viewBox=\"0 0 2187 2048\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");
    // auto p0 = S.back();
    // for (const auto& p1 : S) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    // for (const auto& p : S) {
    //     fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"10\" />\n", p.xcoord(), p.ycoord());
    // }
    // fmt::print("</svg>\n");

    CHECK(!is_anticlockwise);
    CHECK(!rpolygon_is_anticlockwise<int>(S));
    CHECK(rpolygon_is_xmonotone<int>(S));
    CHECK(!rpolygon_is_ymonotone<int>(S));

    const auto P = RPolygon<int>(S);
    CHECK_EQ(P.signed_area(), -2098656);
}

TEST_CASE("Rectilinear Polygon test (ycoord-mono 50)") {
    auto hgenX = ildsgen::VdCorput(3, 7);
    auto hgenY = ildsgen::VdCorput(2, 11);
    auto S = std::vector<Point<int>>{};
    for (auto i = 0; i != 50; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    const auto is_clockwise = create_ymono_rpolygon(S.begin(), S.end());
    CHECK(is_clockwise);
    CHECK(!rpolygon_is_anticlockwise<int>(S));
    CHECK(rpolygon_is_ymonotone<int>(S));
    CHECK(!rpolygon_is_xmonotone<int>(S));
    const auto q = Point<int>(int(hgenX.pop()), int(hgenY.pop()));
    CHECK(!point_in_rpolygon<int>(S, q));

    // SVG output (commented out as per reference)
    // fmt::print("<svg viewBox=\"0 0 2187 2048\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");
    // auto p0 = S.back();
    // for (const auto& p1 : S) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    // for (const auto& p : S) {
    //     fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"10\" />\n", p.xcoord(), p.ycoord());
    // }
    // fmt::print("</svg>\n");

    const auto P = RPolygon<int>(S);
    CHECK_EQ(P.signed_area(), -2032128);
}

TEST_CASE("Rectilinear Polygon point_in_rpolygon test") {
    // Define a square in counter-clockwise order
    const auto S = std::vector<Point<int>>{{0, 0}, {10, 0}, {10, 10}, {0, 10}};

    // Test points strictly inside the polygon
    CHECK(point_in_rpolygon<int>(S, Point<int>{5, 5}));
    CHECK(point_in_rpolygon<int>(S, Point<int>{1, 1}));
    CHECK(point_in_rpolygon<int>(S, Point<int>{9, 9}));

    // Test points strictly outside the polygon
    CHECK(!point_in_rpolygon<int>(S, Point<int>{-1, -1}));
    CHECK(!point_in_rpolygon<int>(S, Point<int>{11, 5}));
    CHECK(!point_in_rpolygon<int>(S, Point<int>{5, -1}));
    CHECK(!point_in_rpolygon<int>(S, Point<int>{5, 11}));
}

TEST_CASE("RPolygon to_polygon conversion") {
    const auto coords = std::vector<Point<int>>{{0, 0}, {10, 10}, {5, 5}};
    const auto r_poly = RPolygon<int>(coords);
    const auto poly = r_poly.to_polygon();

    // Expected coordinates after conversion to maintain rectilinearity
    const auto expected_coords
        = std::vector<Point<int>>{{0, 0}, {10, 0}, {10, 10}, {5, 10}, {5, 5}, {0, 5}};
    const auto expected_poly = Polygon<int>(expected_coords);

    CHECK(poly == expected_poly);
}

TEST_CASE("RPolygon rectilinearity test") {
    // Rectilinear polygon
    const auto rectilinear_coords = std::vector<Point<int>>{{0, 0}, {10, 10}, {5, 5}};
    const auto rectilinear_poly = RPolygon<int>(rectilinear_coords);
    CHECK(rectilinear_poly.is_rectilinear());
}

TEST_CASE("RPolygon equality test") {
    const auto coords1 = std::vector<Point<int>>{{0, 0}, {5, 0}, {5, 5}, {0, 5}};
    const auto coords2 = std::vector<Point<int>>{{0, 0}, {5, 0}, {5, 5}, {0, 5}};

    const auto poly1 = RPolygon<int>(coords1);
    const auto poly2 = RPolygon<int>(coords2);

    CHECK(poly1 == poly2);

    // Test inequality
    auto poly3 = RPolygon<int>(coords1);
    poly3 += Vector2<int>{1, 0};
    CHECK(poly1 != poly3);
}

TEST_CASE("RPolygon vertices access") {
    const auto coords = std::vector<Point<int>>{{0, 0}, {5, 0}, {5, 5}, {0, 5}};
    const auto poly = RPolygon<int>(coords);

    const auto vertices = poly.vertices();
    CHECK(vertices.size() == 4);
    CHECK(vertices[0] == Point<int>{0, 0});
    CHECK(vertices[1] == Point<int>{5, 0});
    CHECK(vertices[2] == Point<int>{5, 5});
    CHECK(vertices[3] == Point<int>{0, 5});
}
