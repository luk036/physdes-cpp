#include <doctest/doctest.h>

#include <ostream>  // for operator<<
#include <recti/interval.hpp>
#include <recti/point.hpp>

using namespace recti;

TEST_CASE("Point") {
    const Point p1(1, 2);
    const Point p2(1, 2);
    const Point p3(2, 3);

    SUBCASE("Construction and Accessors") {
        CHECK(p1.xcoord() == 1);
        CHECK(p1.ycoord() == 2);
    }

    SUBCASE("Comparison") {
        CHECK(p1 == p2);
        CHECK(p1 != p3);
        CHECK(p1 < p3);
    }

    SUBCASE("Arithmetic Operators") {
        const Vector2 v(1, 1);
        Point p4 = p1 + v;
        CHECK(p4.xcoord() == 2);
        CHECK(p4.ycoord() == 3);
        CHECK(p4 == p3);

        p4 -= v;
        CHECK(p4 == p1);

        const Point p5 = p3 - v;
        CHECK(p5 == p1);
    }

    SUBCASE("Flip") {
        const Point p_flipped = p1.flip_xy();
        CHECK(p_flipped.xcoord() == 2);
        CHECK(p_flipped.ycoord() == 1);

        const Point p_flipped_y = p1.flip_y();
        CHECK(p_flipped_y.xcoord() == -1);
        CHECK(p_flipped_y.ycoord() == 2);
    }

    SUBCASE("Overlaps, Contains, Intersects, Hull") {
        const Point p_interval1(Interval(0, 2), Interval(0, 2));
        const Point p_interval2(Interval(1, 3), Interval(1, 3));
        const Point p_interval3(Interval(3, 4), Interval(3, 4));

        CHECK(p_interval1.overlaps(p_interval2));
        CHECK(!p_interval1.overlaps(p_interval3));

        CHECK(p_interval1.contains(Point(1, 1)));
        CHECK(!p_interval1.contains(Point(3, 3)));

        const Point intersection = p_interval1.intersect_with(p_interval2);
        CHECK(intersection.xcoord() == Interval(1, 2));
        CHECK(intersection.ycoord() == Interval(1, 2));

        const Point hull = p_interval1.hull_with(p_interval2);
        CHECK(hull.xcoord() == Interval(0, 3));
        CHECK(hull.ycoord() == Interval(0, 3));
    }

    SUBCASE("Min Distance") {
        const Point p_interval1(Interval(0, 2), Interval(0, 2));
        const Point p_interval2(Interval(4, 5), Interval(4, 5));

        const auto dist = p_interval1.min_dist_with(p_interval2);
        CHECK(dist == 4);
    }
}

TEST_CASE("Point nearest_to") {
    const Point a(3, 4);
    const Point b(5, 6);
    CHECK(a.nearest_to(b) == Point(3, 4));

    const Point r(Interval(3, 4), Interval(5, 6));  // Rectangle
    CHECK(r.nearest_to(a) == Point(3, 5));
    CHECK(r.nearest_to(b) == Point(4, 6));
}

TEST_CASE("Point class") {
    Point<int, int> a(3, 4);

    SUBCASE("Constructor and accessors") {
        CHECK(a.xcoord() == 3);
        CHECK(a.ycoord() == 4);
    }
    SUBCASE("measure") {
        CHECK(a.measure() == 1);
        Point<int, int> b(3, 8);
        CHECK(b.measure() == 1);
    }
    SUBCASE("rotates") {
        auto rotated = a.rotates();
        CHECK(rotated.xcoord() == -1);
        CHECK(rotated.ycoord() == 7);
    }
    SUBCASE("inv_rotates") {
        Point<int, int> b(-1, 9);
        auto inv_rotated = b.inv_rotates();
        CHECK(inv_rotated.xcoord() == 4);
        CHECK(inv_rotated.ycoord() == 5);
    }
    SUBCASE("blocks") {
        Point<int, int> b(5, 6);
        CHECK(a.blocks(b) == false);
        // Add more complex cases with Intervals if needed
    }
    SUBCASE("nearest_to") {
        Point<int, int> b(5, 6);
        auto nearest = a.nearest_to(b);
        CHECK(nearest.xcoord() == 3);
        CHECK(nearest.ycoord() == 4);
        Point<Interval<int>, Interval<int>> r(Interval<int>(3, 4), Interval<int>(5, 6));
        auto nearest1 = r.nearest_to(a);
        CHECK(nearest1.xcoord() == 3);
        CHECK(nearest1.ycoord() == 5);
        auto nearest2 = r.nearest_to(b);
        CHECK(nearest2.xcoord() == 4);
        CHECK(nearest2.ycoord() == 6);
    }
    SUBCASE("get_center") {
        CHECK(a.get_center().xcoord() == 3);
        CHECK(a.get_center().ycoord() == 4);
        Point<Interval<int>, int> b(Interval<int>(3, 7), 4);
        CHECK(b.get_center().xcoord() == 5);
        CHECK(b.get_center().ycoord() == 4);
    }
    SUBCASE("lower_corner") {
        CHECK(a.lower_corner().xcoord() == 3);
        CHECK(a.lower_corner().ycoord() == 4);
        Point<Interval<int>, int> b(Interval<int>(3, 7), 4);
        CHECK(b.lower_corner().xcoord() == 3);
        CHECK(b.lower_corner().ycoord() == 4);
    }
    SUBCASE("upper_corner") {
        CHECK(a.upper_corner().xcoord() == 3);
        CHECK(a.upper_corner().ycoord() == 4);
        Point<Interval<int>, int> b(Interval<int>(3, 7), 4);
        CHECK(b.upper_corner().xcoord() == 7);
        CHECK(b.upper_corner().ycoord() == 4);
    }
}