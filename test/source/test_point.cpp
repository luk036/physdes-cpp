#include <doctest/doctest.h>

#include <ostream>  // for operator<<
#include <recti/interval.hpp>
#include <recti/point.hpp>

using namespace recti;

TEST_CASE("Point") {
    const Point point1(1, 2);
    const Point point2(1, 2);
    const Point point3(2, 3);

    SUBCASE("Construction and Accessors") {
        CHECK(point1.xcoord() == 1);
        CHECK(point1.ycoord() == 2);
    }

    SUBCASE("Comparison") {
        CHECK(point1 == point2);
        CHECK(point1 != point3);
        CHECK(point1 < point3);
    }

    SUBCASE("Arithmetic Operators") {
        const Vector2 vector1(1, 1);
        Point point4 = point1 + vector1;
        CHECK(point4.xcoord() == 2);
        CHECK(point4.ycoord() == 3);
        CHECK(point4 == point3);

        point4 -= vector1;
        CHECK(point4 == point1);

        const Point point5 = point3 - vector1;
        CHECK(point5 == point1);
    }

    SUBCASE("Flip") {
        const Point point_flipped = point1.flip_xy();
        CHECK(point_flipped.xcoord() == 2);
        CHECK(point_flipped.ycoord() == 1);

        const Point point_flipped_y = point1.flip_y();
        CHECK(point_flipped_y.xcoord() == -1);
        CHECK(point_flipped_y.ycoord() == 2);
    }

    SUBCASE("Overlaps, Contains, Intersects, Hull") {
        const Point point_interval1(Interval(0, 2), Interval(0, 2));
        const Point point_interval2(Interval(1, 3), Interval(1, 3));
        const Point point_interval3(Interval(3, 4), Interval(3, 4));

        CHECK(point_interval1.overlaps(point_interval2));
        CHECK(!point_interval1.overlaps(point_interval3));

        CHECK(point_interval1.contains(Point(1, 1)));
        CHECK(!point_interval1.contains(Point(3, 3)));

        const Point intersection = point_interval1.intersect_with(point_interval2);
        CHECK(intersection.xcoord() == Interval(1, 2));
        CHECK(intersection.ycoord() == Interval(1, 2));

        const Point hull = point_interval1.hull_with(point_interval2);
        CHECK(hull.xcoord() == Interval(0, 3));
        CHECK(hull.ycoord() == Interval(0, 3));
    }

    SUBCASE("Min Distance") {
        const Point point_interval1(Interval(0, 2), Interval(0, 2));
        const Point point_interval2(Interval(4, 5), Interval(4, 5));

        const auto dist = point_interval1.min_dist_with(point_interval2);
        CHECK(dist == 4);
    }
}

TEST_CASE("Point nearest_to") {
    const Point point_a(3, 4);
    const Point point_b(5, 6);
    CHECK(point_a.nearest_to(point_b) == Point(3, 4));

    const Point rectangle(Interval(3, 4), Interval(5, 6));  // Rectangle
    CHECK(rectangle.nearest_to(point_a) == Point(3, 5));
    CHECK(rectangle.nearest_to(point_b) == Point(4, 6));
}

TEST_CASE("Point class") {
    Point<int, int> point_a(3, 4);

    SUBCASE("Constructor and accessors") {
        CHECK(point_a.xcoord() == 3);
        CHECK(point_a.ycoord() == 4);
    }
    SUBCASE("measure") {
        CHECK(point_a.measure() == 1);
        Point<int, int> point_b(3, 8);
        CHECK(point_b.measure() == 1);
    }
    SUBCASE("rotates") {
        auto rotated = point_a.rotates();
        CHECK(rotated.xcoord() == -1);
        CHECK(rotated.ycoord() == 7);
    }
    SUBCASE("inv_rotates") {
        Point<int, int> point_b(-1, 9);
        auto inv_rotated = point_b.inv_rotates();
        CHECK(inv_rotated.xcoord() == 4);
        CHECK(inv_rotated.ycoord() == 5);
    }
    SUBCASE("blocks") {
        Point<int, int> point_b(5, 6);
        CHECK(point_a.blocks(point_b) == false);
        // Add more complex cases with Intervals if needed
    }
    SUBCASE("nearest_to") {
        Point<int, int> point_b(5, 6);
        auto nearest = point_a.nearest_to(point_b);
        CHECK(nearest.xcoord() == 3);
        CHECK(nearest.ycoord() == 4);
        Point<Interval<int>, Interval<int>> rectangle(Interval<int>(3, 4), Interval<int>(5, 6));
        auto nearest1 = rectangle.nearest_to(point_a);
        CHECK(nearest1.xcoord() == 3);
        CHECK(nearest1.ycoord() == 5);
        auto nearest2 = rectangle.nearest_to(point_b);
        CHECK(nearest2.xcoord() == 4);
        CHECK(nearest2.ycoord() == 6);
    }
    SUBCASE("get_center") {
        CHECK(point_a.get_center().xcoord() == 3);
        CHECK(point_a.get_center().ycoord() == 4);
        Point<Interval<int>, int> point_b(Interval<int>(3, 7), 4);
        CHECK(point_b.get_center().xcoord() == 5);
        CHECK(point_b.get_center().ycoord() == 4);
    }
    SUBCASE("lower_corner") {
        CHECK(point_a.lower_corner().xcoord() == 3);
        CHECK(point_a.lower_corner().ycoord() == 4);
        Point<Interval<int>, int> point_b(Interval<int>(3, 7), 4);
        CHECK(point_b.lower_corner().xcoord() == 3);
        CHECK(point_b.lower_corner().ycoord() == 4);
    }
    SUBCASE("upper_corner") {
        CHECK(point_a.upper_corner().xcoord() == 3);
        CHECK(point_a.upper_corner().ycoord() == 4);
        Point<Interval<int>, int> point_b(Interval<int>(3, 7), 4);
        CHECK(point_b.upper_corner().xcoord() == 7);
        CHECK(point_b.upper_corner().ycoord() == 4);
    }
}