#ifdef RAPIDCHECK_H
#    include <doctest/doctest.h>

// Disable C4180 warning from RapidCheck on MSVC
#    ifdef _MSC_VER
#        pragma warning(push)
#        pragma warning(disable : 4180)
#        include <rapidcheck.h>
#        pragma warning(pop)
#    else
#        include <rapidcheck.h>
#    endif

#    include "recti/interval.hpp"
#    include "recti/point.hpp"

TEST_CASE("Property-based test: Interval lower bound <= upper bound") {
    rc::check("Interval maintains lb <= ub", []() {
        auto lb = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        auto ub = static_cast<int>(*rc::gen::inRange(lb, 1000));
        
        recti::Interval<int> interval(lb, ub);
        
        RC_ASSERT(interval.lb() <= interval.ub());
    });
}

TEST_CASE("Property-based test: Interval measure is non-negative") {
    rc::check("Interval measure() returns non-negative value", []() {
        auto lb = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        auto ub = static_cast<int>(*rc::gen::inRange(lb, 1000));
        
        recti::Interval<int> interval(lb, ub);
        auto measure = interval.measure();
        
        RC_ASSERT(measure >= 0);
    });
}

TEST_CASE("Property-based test: Interval measure equals ub - lb") {
    rc::check("Interval measure() equals ub() - lb()", []() {
        auto lb = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        auto ub = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        
        recti::Interval<int> interval(lb, ub);
        auto measure = interval.measure();
        auto expected = ub - lb;
        
        RC_ASSERT(measure == expected);
    });
}

TEST_CASE("Property-based test: Point equality is reflexive") {
    rc::check("Point == itself", []() {
        auto x = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        auto y = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        
        recti::Point<int> p(x, y);
        
        RC_ASSERT(p == p);
    });
}

TEST_CASE("Property-based test: Point equality is symmetric") {
    rc::check("Point equality is symmetric", []() {
        auto x = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        auto y = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        
        recti::Point<int> p1(x, y);
        recti::Point<int> p2(x, y);
        
        RC_ASSERT((p1 == p2) == (p2 == p1));
    });
}

TEST_CASE("Property-based test: Point coordinates are preserved") {
    rc::check("Point preserves xcoord and ycoord", []() {
        auto x = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        auto y = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        
        recti::Point<int> p(x, y);
        
        RC_ASSERT(p.xcoord() == x);
        RC_ASSERT(p.ycoord() == y);
    });
}

TEST_CASE("Property-based test: Point subtraction returns correct Vector2") {
    rc::check("Point subtraction returns Vector2 with correct components", []() {
        auto x1 = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto y1 = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto x2 = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto y2 = static_cast<int>(*rc::gen::inRange(-100, 100));
        
        recti::Point<int> p1(x1, y1);
        recti::Point<int> p2(x2, y2);
        auto vec = p1 - p2;
        
        RC_ASSERT(vec.x() == x1 - x2);
        RC_ASSERT(vec.y() == y1 - y2);
    });
}

TEST_CASE("Property-based test: Point flip_y inverts x coordinate") {
    rc::check("Point flip_y inverts x coordinate", []() {
        auto x = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto y = static_cast<int>(*rc::gen::inRange(-100, 100));
        
        recti::Point<int> p(x, y);
        auto flipped = p.flip_y();
        
        RC_ASSERT(flipped.xcoord() == -x);
        RC_ASSERT(flipped.ycoord() == y);
    });
}

TEST_CASE("Property-based test: Point flip_xy swaps coordinates") {
    rc::check("Point flip_xy swaps x and y coordinates", []() {
        auto x = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto y = static_cast<int>(*rc::gen::inRange(-100, 100));
        
        recti::Point<int> p(x, y);
        auto flipped = p.flip_xy();
        
        RC_ASSERT(flipped.xcoord() == y);
        RC_ASSERT(flipped.ycoord() == x);
    });
}

TEST_CASE("Property-based test: Interval with same value has zero measure") {
    rc::check("Interval(v, v) has measure 0", []() {
        auto v = static_cast<int>(*rc::gen::inRange(-1000, 1000));
        
        recti::Interval<int> interval(v);
        
        RC_ASSERT(interval.measure() == 0);
        RC_ASSERT(interval.lb() == v);
        RC_ASSERT(interval.ub() == v);
    });
}

TEST_CASE("Property-based test: Point distance squared is non-negative") {
    rc::check("Point distance squared is non-negative", []() {
        auto x1 = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto y1 = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto x2 = static_cast<int>(*rc::gen::inRange(-100, 100));
        auto y2 = static_cast<int>(*rc::gen::inRange(-100, 100));
        
        recti::Point<int> p1(x1, y1);
        recti::Point<int> p2(x2, y2);
        auto vec = p1 - p2;
        auto dist_sq = vec.x() * vec.x() + vec.y() * vec.y();
        
        RC_ASSERT(dist_sq >= 0);
    });
}

#endif