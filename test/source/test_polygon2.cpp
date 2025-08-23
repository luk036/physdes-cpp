#include <doctest/doctest.h>
#include <recti/polygon.hpp>
#include <vector>
#include "recti/point.hpp"

using namespace recti;

TEST_CASE("Polygon test (ycoord-mono) 2") {
    auto S = std::vector<Point<int>>{
        {0, 0}, {0, 10}, {10, 10}, {10, 0}
    };
    create_ymono_polygon(S.begin(), S.end());
    auto P = Polygon<int>(S);
    CHECK(P.signed_area_x2() == 200);
    CHECK(!polygon_is_clockwise<int>(S));
    CHECK(point_in_polygon<int>(S, Point<int>{5, 5}));
    CHECK(!point_in_polygon<int>(S, Point<int>{15, 5}));
}