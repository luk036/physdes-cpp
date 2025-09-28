#include <doctest/doctest.h>
#include <fmt/core.h>

#include <ldsgen/ilds.hpp>          // for VdCorput
#include <recti/point.hpp>          // for Point
#include <recti/rpolygon.hpp>       // for create_xmono_rpolygon, create_test_rpolygon
#include <recti/rpolygon_hull.hpp>  // for rpolygon_is_monotone, rpolygon_make_xmonotone_hull, etc.
#include <span>
#include <vector>

using namespace recti;

TEST_CASE("RPolygon is_monotone small list test") {
    std::vector<Point<int>> points = {{0, 0}, {1, 1}};
    CHECK(rpolygon_is_monotone<int>(
        points, [](const Point<int>& p) { return std::make_pair(p.xcoord(), p.ycoord()); }));
}

TEST_CASE("RPolygon is_monotone break test") {
    std::vector<Point<int>> points = {{0, 0}, {3, 1}, {1, 2}, {2, 3}};
    CHECK_FALSE(rpolygon_is_monotone<int>(
        points, [](const Point<int>& p) { return std::make_pair(p.xcoord(), p.ycoord()); }));
}

TEST_CASE("RPolygon make x-monotone hull test") {
    std::vector<Point<int>> S = {{-10, 50}, {-40, 40}, {-60, -40}, {-20, -50}, {90, -2},
                                 {60, 10},  {50, 20},  {10, 40},   {80, 60}};
    CHECK_FALSE(rpolygon_is_xmonotone<int>(S));

    // // SVG output (commented out as per reference test_rpolygon.cpp)
    // fmt::print("<svg viewBox=\"-100 -100 200 200\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");
    // auto p0 = S.back();
    // for (const auto& p1 : S) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    // for (const auto& p : S) {
    //     fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"1\" />\n", p.xcoord(), p.ycoord());
    // }

    auto C = rpolygon_make_xmonotone_hull<int>(S, true);

    // // SVG output for hull
    // fmt::print("  <polygon points=\"");
    // p0 = C.back();
    // for (const auto& p1 : C) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // fmt::print("</svg>\n");

    CHECK(rpolygon_is_xmonotone<int>(C));
}

TEST_CASE("RPolygon make y-monotone hull test") {
    std::vector<Point<int>> S = {{90, -10}, {40, -40}, {-40, -60}, {-50, -20}, {-20, 90},
                                 {10, 60},  {20, 50},  {30, 10},   {60, 80}};
    CHECK_FALSE(rpolygon_is_ymonotone<int>(S));

    // // SVG output (commented out as per reference)
    // fmt::print("<svg viewBox=\"-100 -100 200 200\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");
    // auto p0 = S.back();
    // for (const auto& p1 : S) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    // for (const auto& p : S) {
    //     fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"1\" />\n", p.xcoord(), p.ycoord());
    // }

    auto C = rpolygon_make_ymonotone_hull<int>(S, false);

    // // SVG output for hull
    // fmt::print("  <polygon points=\"");
    // p0 = C.back();
    // for (const auto& p1 : C) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // fmt::print("</svg>\n");

    CHECK(rpolygon_is_ymonotone<int>(C));
}

TEST_CASE("RPolygon make convex hull test with Halton points") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int>> S;
    for (int i = 0; i < 100; ++i) {
        S.emplace_back(Point<int>(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop())));
    }
    auto P = create_test_rpolygon(S.begin(), S.end());
    CHECK_FALSE(rpolygon_is_xmonotone<int>(P));
    CHECK_FALSE(rpolygon_is_ymonotone<int>(P));
    auto C = rpolygon_make_convex_hull<int>(P, false);

    // // SVG output (commented out as per reference)
    // fmt::print("<svg viewBox=\"0 0 2187 2048\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");
    // auto p0 = P.back();
    // for (const auto& p1 : P) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    // for (const auto& p : P) {
    //     fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"10\" />\n", p.xcoord(), p.ycoord());
    // }

    // // SVG output for hull
    // fmt::print("  <polygon points=\"");
    // p0 = C.back();
    // for (const auto& p1 : C) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // fmt::print("</svg>\n");

    CHECK(rpolygon_is_convex<int>(C));
}

TEST_CASE("RPolygon make xmono hull test") {
    auto hgenX = ildsgen::VdCorput(3, 7);
    auto hgenY = ildsgen::VdCorput(2, 11);
    auto S = std::vector<Point<int>>{};
    for (auto i = 0; i != 50; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    auto is_clockwise = create_ymono_rpolygon(S.begin(), S.end());
    CHECK(rpolygon_is_ymonotone<int>(S));
    CHECK_FALSE(rpolygon_is_xmonotone<int>(S));

    // // SVG output (commented out as per reference)
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

    auto C = rpolygon_make_xmonotone_hull<int>(S, !is_clockwise);

    // // SVG output for hull
    // fmt::print("  <polygon points=\"");
    // p0 = C.back();
    // for (const auto& p1 : C) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // fmt::print("</svg>\n");

    CHECK(rpolygon_is_xmonotone<int>(C));

    // CHECK(rpolygon_is_convex<int>(C));
}

TEST_CASE("RPolygon make ymono hull test") {
    auto hgenX = ildsgen::VdCorput(3, 7);
    auto hgenY = ildsgen::VdCorput(2, 11);
    auto S = std::vector<Point<int>>{};
    for (auto i = 0; i != 50; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    auto is_anticlockwise = create_xmono_rpolygon(S.begin(), S.end());
    CHECK(rpolygon_is_xmonotone<int>(S));
    CHECK_FALSE(rpolygon_is_ymonotone<int>(S));

    // // SVG output (commented out as per reference)
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

    auto C = rpolygon_make_ymonotone_hull<int>(S, is_anticlockwise);

    // // SVG output for hull
    // fmt::print("  <polygon points=\"");
    // p0 = C.back();
    // for (const auto& p1 : C) {
    //     fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //     p0 = p1;
    // }
    // fmt::print("\"\n");
    // fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // fmt::print("</svg>\n");

    CHECK(rpolygon_is_ymonotone<int>(C));

    // CHECK(rpolygon_is_convex<int>(C));
}