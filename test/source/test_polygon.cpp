#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, Expr...

#include <algorithm>        // for generate
#include <functional>       // for function
#include <ldsgen/ilds.hpp>  // for VdCorput
#include <ostream>
#include <recti/polygon.hpp>  // for Polygon, create_ymono_polygon, create_xmono_polygon, etc.
#include <recti/rpolygon.hpp>  // for RPolygon
#include <recti/rpolygon_cut.hpp>  // for rpolygon_cut_convex, etc.
#include <recti/rpolygon_hull.hpp>  // for rpolygon_make_convex_hull, etc.
#include <vector>             // for vector

#include "recti/point.hpp"  // for Point

using namespace recti;

TEST_CASE("Polygon y-monotone") {
    auto S
        = std::vector<Point<int>>{{-2, 2}, {0, -1}, {-5, 1}, {-2, 4},  {0, -4}, {-4, 3},  {-6, -2},
                                  {5, 1},  {2, 2},  {3, -3}, {-3, -3}, {3, 3},  {-3, -4}, {1, 4}};

    create_ymono_polygon(S.begin(), S.end());
    CHECK(polygon_is_ymonotone<int>(S));
    CHECK(!polygon_is_xmonotone<int>(S));
    CHECK(polygon_is_anticlockwise<int>(S));
    const auto P = Polygon<int>(S);
    CHECK(P.signed_area_x2() == 102);
    CHECK(!point_in_polygon<int>(S, Point<int>{4, 5}));

    // Test += and -= operators
    auto Q = Polygon<int>(S);
    Q += Vector2<int>{4, 5};
    Q -= Vector2<int>{4, 5};
    CHECK(Q == P);
}

TEST_CASE("Polygon x-monotone") {
    auto S
        = std::vector<Point<int>>{{-2, 2}, {0, -1}, {-5, 1}, {-2, 4},  {0, -4}, {-4, 3},  {-6, -2},
                                  {5, 1},  {2, 2},  {3, -3}, {-3, -3}, {3, 3},  {-3, -4}, {1, 4}};

    create_xmono_polygon(S.begin(), S.end());
    CHECK(polygon_is_xmonotone<int>(S));
    CHECK(!polygon_is_ymonotone<int>(S));
    CHECK(polygon_is_anticlockwise<int>(S));
    const auto P = Polygon<int>(S);
    CHECK(P.signed_area_x2() == 111);
}

TEST_CASE("Polygon y-monotone (20 points)") {
    auto hgenX = ildsgen::VdCorput(2, 11);
    auto hgenY = ildsgen::VdCorput(3, 7);

    auto S = std::vector<Point<int>>{};
    for (auto i = 0U; i != 20; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }

    create_ymono_polygon(S.begin(), S.end());
    CHECK(polygon_is_ymonotone<int>(S));
    CHECK(!polygon_is_xmonotone<int>(S));
    CHECK(polygon_is_anticlockwise<int>(S));
    const auto P = Polygon<int>(S);
    CHECK(P.signed_area_x2() == 4074624);
}

TEST_CASE("Polygon x-monotone (20 points)") {
    auto hgenX = ildsgen::VdCorput(2, 11);
    auto hgenY = ildsgen::VdCorput(3, 7);

    auto S = std::vector<Point<int>>{};
    for (auto i = 0U; i != 20; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }

    create_xmono_polygon(S.begin(), S.end());
    CHECK(polygon_is_xmonotone<int>(S));
    CHECK(!polygon_is_ymonotone<int>(S));
    CHECK(polygon_is_anticlockwise<int>(S));
    const auto P = Polygon<int>(S);
    CHECK(P.signed_area_x2() == 3862080);
}

TEST_CASE("Polygon y-monotone (50 points)") {
    auto hgenX = ildsgen::VdCorput(3, 7);
    auto hgenY = ildsgen::VdCorput(2, 11);

    auto S = std::vector<Point<int>>{};
    for (auto i = 0U; i != 50; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }

    create_ymono_polygon(S.begin(), S.end());
    CHECK(polygon_is_ymonotone<int>(S));
    CHECK(!polygon_is_xmonotone<int>(S));
    CHECK(polygon_is_anticlockwise<int>(S));
    const auto q = Point<int>(int(hgenX.pop()), int(hgenY.pop()));
    const auto P = Polygon<int>(S);
    CHECK(P.signed_area_x2() == 4409856);
    CHECK(point_in_polygon<int>(S, q));
}

TEST_CASE("Polygon is_rectilinear") {
    // Create a rectilinear polygon
    const auto rectilinear_coords = std::vector<Point<int>>{{0, 0}, {0, 1}, {1, 1}, {1, 0}};
    const auto rectilinear_polygon = Polygon<int>(rectilinear_coords);
    CHECK(rectilinear_polygon.is_rectilinear() == true);

    // Create a non-rectilinear polygon
    const auto non_rectilinear_coords = std::vector<Point<int>>{{0, 0}, {1, 1}, {2, 0}};
    const auto non_rectilinear_polygon = Polygon<int>(non_rectilinear_coords);
    CHECK(non_rectilinear_polygon.is_rectilinear() == false);
}

TEST_CASE("Polygon is_convex") {
    // Test case 1: Convex polygon
    const auto convex_coords = std::vector<Point<int>>{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    const auto convex_polygon = Polygon<int>(convex_coords);
    CHECK(convex_polygon.is_convex() == true);

    // Test case 2: Non-convex polygon
    const auto non_convex_coords = std::vector<Point<int>>{{0, 0}, {2, 0}, {1, 1}, {2, 2}, {0, 2}};
    const auto non_convex_polygon = Polygon<int>(non_convex_coords);
    CHECK(non_convex_polygon.is_convex() == false);

    // Test case 3: Triangle (always convex)
    const auto triangle_coords = std::vector<Point<int>>{{0, 0}, {2, 0}, {1, 2}};
    const auto triangle = Polygon<int>(triangle_coords);
    CHECK(triangle.is_convex() == true);
}

TEST_CASE("Polygon equality operator") {
    const auto coords = std::vector<Point<int>>{{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    const auto P = Polygon<int>(coords);
    const auto Q = Polygon<int>(coords);
    CHECK(P == Q);

    // Test inequality
    auto R = Polygon<int>(coords);
    R += Vector2<int>{1, 0};
    CHECK(P != R);
}

TEST_CASE("Polygon vertices access") {
    const auto coords = std::vector<Point<int>>{{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    const auto P = Polygon<int>(coords);

    const auto vertices = P.vertices();
    CHECK(vertices.size() == 4);
    CHECK(vertices[0] == Point<int>{0, 0});
    CHECK(vertices[1] == Point<int>{1, 0});
    CHECK(vertices[2] == Point<int>{1, 1});
    CHECK(vertices[3] == Point<int>{0, 1});
}

TEST_CASE("Polygon empty and small cases") {
    // Single point
    const auto single_coords = std::vector<Point<int>>{{1, 1}};
    const auto single_polygon = Polygon<int>(single_coords);
    CHECK(single_polygon.signed_area_x2() == 0);
    CHECK(single_polygon.is_rectilinear() == true);
    CHECK(single_polygon.is_convex() == false);

    // Two points (line segment)
    const auto line_coords = std::vector<Point<int>>{{0, 0}, {1, 1}};
    const auto line_polygon = Polygon<int>(line_coords);
    CHECK(line_polygon.signed_area_x2() == 0);
    CHECK(line_polygon.is_rectilinear() == false);  // Diagonal line is not rectilinear
    CHECK(line_polygon.is_convex() == false);
}

TEST_CASE("Polygon y-monotone (square)") {
    auto S = std::vector<Point<int>>{{0, 0}, {0, 10}, {10, 10}, {10, 0}};
    create_ymono_polygon(S.begin(), S.end());
    const auto P = Polygon<int>(S);
    CHECK(P.signed_area_x2() == 200);
    CHECK(polygon_is_anticlockwise<int>(S));
    CHECK(point_in_polygon<int>(S, Point<int>{5, 5}));
    CHECK(!point_in_polygon<int>(S, Point<int>{15, 5}));
}

TEST_CASE("Polygon convex decomposition - signed area preservation") {
    // Test with a random non-convex polygon created by test_rpolygon
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int>> S;
    for (int i = 0; i < 10; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    const auto P = create_test_rpolygon(S.begin(), S.end());
    const bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
    
    // Verify the original polygon is not convex
    CHECK(!rpolygon_is_convex<int>(P));
    
    // Perform convex decomposition
    const auto convex_pieces = rpolygon_cut_convex<int>(P, is_anticlockwise);
    
    // Verify we have at least one piece
    CHECK(convex_pieces.size() > 0);
    
    // Verify all pieces are convex
    for (const auto& piece : convex_pieces) {
        CHECK(rpolygon_is_convex<int>(piece));
    }
    
    // Calculate sum of signed areas of all pieces
    int total_pieces_area = 0;
    for (const auto& piece : convex_pieces) {
        const auto piece_rpolygon = RPolygon<int>(piece);
        total_pieces_area += piece_rpolygon.signed_area();
    }
    
    // Verify signed area preservation by comparing with the original RPolygon area
    const auto original_rpolygon = RPolygon<int>(P);
    const int original_rpolygon_area = original_rpolygon.signed_area();
    CHECK(original_rpolygon_area == total_pieces_area);
}

TEST_CASE("Polygon explicit cut - signed area preservation") {
    // Test with a random non-convex polygon created by test_rpolygon
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int>> S;
    for (int i = 0; i < 10; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    const auto P = create_test_rpolygon(S.begin(), S.end());
    const bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
    
    // First create convex hull
    const auto convex_hull = rpolygon_make_convex_hull<int>(P, is_anticlockwise);
    
    // Perform explicit decomposition
    const auto explicit_pieces = rpolygon_cut_explicit<int>(convex_hull, is_anticlockwise);
    
    // Verify we have at least one piece
    CHECK(explicit_pieces.size() > 0);
    
    // Verify all pieces are convex
    for (const auto& piece : explicit_pieces) {
        CHECK(rpolygon_is_convex<int>(piece));
    }
    
    // Calculate sum of signed areas of all pieces
    int total_pieces_area = 0;
    for (const auto& piece : explicit_pieces) {
        const auto piece_rpolygon = RPolygon<int>(piece);
        total_pieces_area += piece_rpolygon.signed_area();
    }
    
    // Verify signed area preservation
    const auto hull_rpolygon = RPolygon<int>(convex_hull);
    const int hull_area = hull_rpolygon.signed_area();
    CHECK(hull_area == total_pieces_area);
}

TEST_CASE("Polygon implicit cut - signed area preservation") {
    // Test with a random non-convex polygon created by test_rpolygon
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    std::vector<Point<int>> S;
    for (int i = 0; i < 10; ++i) {
        S.emplace_back(Point<int>(int(hgenX.pop()), int(hgenY.pop())));
    }
    const auto P = create_test_rpolygon(S.begin(), S.end());
    const bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
    
    // First create convex hull
    const auto convex_hull = rpolygon_make_convex_hull<int>(P, is_anticlockwise);
    
    // Perform implicit decomposition
    const auto implicit_pieces = rpolygon_cut_implicit<int>(convex_hull, is_anticlockwise);
    
    // Verify we have at least one piece
    CHECK(implicit_pieces.size() > 0);
    
    // Verify all pieces are convex
    for (const auto& piece : implicit_pieces) {
        CHECK(rpolygon_is_convex<int>(piece));
    }
    
    // Calculate sum of signed areas of all pieces
    int total_pieces_area = 0;
    for (const auto& piece : implicit_pieces) {
        const auto piece_rpolygon = RPolygon<int>(piece);
        total_pieces_area += piece_rpolygon.signed_area();
    }
    
    // Verify signed area preservation
    const auto hull_rpolygon = RPolygon<int>(convex_hull);
    const int hull_area = hull_rpolygon.signed_area();
    CHECK(hull_area == total_pieces_area);
}