#include <doctest/doctest.h>
#include <fmt/core.h>

#include <ldsgen/ilds.hpp>     // for VdCorput
#include <recti/point.hpp>     // for Point
#include <recti/rpolygon.hpp>  // for create_xmono_rpolygon, create_test_rpolygon
#include <recti/rpolygon_cut.hpp>
#include <recti/rpolygon_hull.hpp>  // for rpolygon_is_monotone, rpolygon_make_xmonotone_hull, etc.
#include <span>
#include <vector>

using namespace recti;

TEST_CASE("RPolygon convex cut test") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int, int>> S;
    for (int i = 0; i < 40; ++i) {
        S.emplace_back(Point<int>(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop())));
    }
    auto P = create_test_rpolygon(S.begin(), S.end());
    // CHECK((!rpolygon_is_xmonotone<int>(P) || !rpolygon_is_ymonotone<int>(P)));
    bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
    // CHECK_FALSE(is_anticlockwise);

    // // SVG output (commented out as per reference)
    // fmt::print("<svg viewBox=\"0 0 2187 2048\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");

    // {
    //     auto p0 = P.back();
    //     for (const auto& p1 : P) {
    //         fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //         p0 = p1;
    //     }
    //     fmt::print("\"\n");
    //     fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    //     for (const auto& p : P) {
    //         fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"10\" />\n", p.xcoord(), p.ycoord());
    //     }
    // }
    auto L = rpolygon_cut_convex<int>(P, is_anticlockwise);

    for (const auto& C : L) {
        CHECK(rpolygon_is_convex<int>(C));
    }

    // // SVG output for convex polygons
    // for (const auto& C : L) {
    //     fmt::print("  <polygon points=\"");
    //     auto p0 = C.back();
    //     for (const auto& p1 : C) {
    //         fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //         p0 = p1;
    //     }
    //     fmt::print("\"\n");
    //     fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // }

    // fmt::print("</svg>\n");
}

TEST_CASE("RPolygon explicit cut test") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int, int>> S;
    for (int i = 0; i < 7; ++i) {
        S.emplace_back(Point<int>(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop())));
    }
    auto P = create_test_rpolygon(S.begin(), S.end());
    // CHECK((!rpolygon_is_xmonotone<int>(P) || !rpolygon_is_ymonotone<int>(P)));
    bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
    auto Q = rpolygon_make_convex_hull<int>(P, is_anticlockwise);

    // SVG output (commented out as per reference)
    // fmt::print("<svg viewBox=\"0 0 2187 2048\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");

    // {
    //     auto p0 = Q.back();
    //     for (const auto& p1 : Q) {
    //         fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //         p0 = p1;
    //     }
    //     fmt::print("\"\n");
    //     fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    //     for (const auto& p : Q) {
    //         fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"10\" />\n", p.xcoord(), p.ycoord());
    //     }
    // }
    auto L = rpolygon_cut_explicit<int>(Q, is_anticlockwise);

    for (const auto& C : L) {
        CHECK(rpolygon_is_convex<int>(C));
    }

    // SVG output for convex polygons
    // for (const auto& C : L) {
    //     fmt::print("  <polygon points=\"");
    //     auto p0 = C.back();
    //     for (const auto& p1 : C) {
    //         fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //         p0 = p1;
    //     }
    //     fmt::print("\"\n");
    //     fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // }

    // fmt::print("</svg>\n");

    // for (const auto& C : L) {
    //     CHECK(rpolygon_is_convex<int>(C));
    // }
}

TEST_CASE("RPolygon implicit cut test") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int, int>> S;
    for (int i = 0; i < 20; ++i) {
        S.emplace_back(Point<int>(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop())));
    }
    auto P = create_test_rpolygon(S.begin(), S.end());
    // CHECK((!rpolygon_is_xmonotone<int>(P) || !rpolygon_is_ymonotone<int>(P)));
    bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
    auto Q = rpolygon_make_convex_hull<int>(P, is_anticlockwise);

    // SVG output (commented out as per reference)
    fmt::print("<svg viewBox=\"0 0 2187 2048\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    fmt::print("  <polygon points=\"");

    {
        auto p0 = Q.back();
        for (const auto& p1 : Q) {
            fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
            p0 = p1;
        }
        fmt::print("\"\n");
        fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
        for (const auto& p : Q) {
            fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"10\" />\n", p.xcoord(), p.ycoord());
        }
    }
    auto L = rpolygon_cut_implicit<int>(Q, is_anticlockwise);

    for (const auto& C : L) {
        CHECK(rpolygon_is_convex<int>(C));
    }

    // SVG output for convex polygons
    for (const auto& C : L) {
        fmt::print("  <polygon points=\"");
        auto p0 = C.back();
        for (const auto& p1 : C) {
            fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
            p0 = p1;
        }
        fmt::print("\"\n");
        fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    }
    fmt::print("</svg>\n");

    // for (const auto& C : L) {
    //     CHECK(rpolygon_is_convex<int>(C));
    // }
}

TEST_CASE("RPolygon cut rectangle test") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int, int>> S;
    for (int i = 0; i < 7; ++i) {
        S.emplace_back(Point<int>(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop())));
    }
    auto P = create_test_rpolygon(S.begin(), S.end());
    // CHECK((!rpolygon_is_xmonotone<int>(P) || !rpolygon_is_ymonotone<int>(P)));
    bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
    auto Q = rpolygon_make_convex_hull<int>(P, is_anticlockwise);

    // // SVG output (commented out as per reference)
    // fmt::print("<svg viewBox=\"0 0 2187 2048\" xmlns=\"http://www.w3.org/2000/svg\">\n");
    // fmt::print("  <polygon points=\"");

    // {
    //     auto p0 = Q.back();
    //     for (const auto& p1 : Q) {
    //         fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //         p0 = p1;
    //     }
    //     fmt::print("\"\n");
    //     fmt::print("  fill=\"#88C0D0\" stroke=\"black\" opacity=\"0.5\"/>\n");
    //     for (const auto& p : Q) {
    //         fmt::print("  <circle cx=\"{}\" cy=\"{}\" r=\"10\" />\n", p.xcoord(), p.ycoord());
    //     }
    // }
    auto L = rpolygon_cut_rectangle<int>(Q, is_anticlockwise);

    for (const auto& C : L) {
        CHECK(rpolygon_is_convex<int>(C));
    }

    // // SVG output for convex polygons
    // for (const auto& C : L) {
    //     fmt::print("  <polygon points=\"");
    //     auto p0 = C.back();
    //     for (const auto& p1 : C) {
    //         fmt::print("{},{} {},{} ", p0.xcoord(), p0.ycoord(), p1.xcoord(), p0.ycoord());
    //         p0 = p1;
    //     }
    //     fmt::print("\"\n");
    //     fmt::print("  fill=\"#D088C0\" stroke=\"black\" opacity=\"0.3\"/>\n");
    // }

    // fmt::print("</svg>\n");

    // for (const auto& C : L) {
    //     CHECK(rpolygon_is_convex<int>(C));
    // }
}
