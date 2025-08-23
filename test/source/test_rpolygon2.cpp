#include <doctest/doctest.h>
#include <recti/rpolygon.hpp>
#include <vector>
#include "recti/point.hpp"

using namespace recti;

TEST_CASE("Rectilinear Polygon point_in_rpolygon test") {
    // Define a square in counter-clockwise order
    auto S = std::vector<Point<int>>{
        {0, 0}, {10, 0}, {10, 10}, {0, 10}
    };

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