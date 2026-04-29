#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <vector>              // for vector

#include <recti/to_polygon.hpp>  // for RPolygon, RPolygon_is_clockwise, cre...
#include <recti/point.hpp>  // for Point, operator>

using namespace recti;

TEST_CASE("RPolygon to_polygon conversion") {
    const auto coords = std::vector<Point<int>>{{0, 0}, {10, 10}, {5, 5}};
    const auto r_poly = RPolygon<int>(coords);
    const auto poly = to_polygon(r_poly);

    // Expected coordinates after conversion to maintain rectilinearity
    const auto expected_coords
        = std::vector<Point<int>>{{0, 0}, {10, 0}, {10, 10}, {5, 10}, {5, 5}, {0, 5}};
    const auto expected_poly = Polygon<int>(expected_coords);

    CHECK(poly == expected_poly);
}
