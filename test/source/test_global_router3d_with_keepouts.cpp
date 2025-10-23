#include <doctest/doctest.h>
#include <fmt/core.h>

#include <iostream>         // for operator<<
#include <ldsgen/ilds.hpp>  // for VdCorput
#include <recti/global_router.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <string>
#include <vector>

using namespace recti;

TEST_CASE("Test route3d_with_steiner_and_keepouts") {
    constexpr int scale_z = 100;

    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    hgenX.reseed(19);
    hgenY.reseed(19);

    using IntPoint = Point<Point<int, int>, int>;
    using Keepout = decltype(std::declval<IntPoint>().enlarge_with(1));

    std::vector<IntPoint> terminals;
    for (int i = 0; i < 7; ++i) {
        terminals.emplace_back(Point{Point{static_cast<int>(hgenX.pop()), (i % 4) * scale_z},
                                     static_cast<int>(hgenY.pop())});
    }
    IntPoint source(Point{Point{static_cast<int>(hgenX.pop()), 0}, static_cast<int>(hgenY.pop())});

    std::vector<Keepout> keepouts = {
        {Point{Interval<int>(1600, 1900), Interval<int>(-1000, 1000)}, Interval<int>(1000, 1500)},
        {Point{Interval<int>(500, 800), Interval<int>(-1000, 1000)}, Interval<int>(600, 900)},
    };

    GlobalRouter<IntPoint> router(source, terminals, keepouts);
    router.route_with_steiners();

    std::string svg_output
        = visualize_routing_tree3d_svg(router.get_tree(), keepouts, scale_z, 1000, 1000);
    std::cout << svg_output;

    save_routing_tree3d_svg(router.get_tree(), keepouts, scale_z,
                            "example_route3d_with_steiner_and_keepouts.svg");
}

TEST_CASE("Test route3d_with_constraints_and_keepouts") {
    constexpr int scale_z = 100;

    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    hgenX.reseed(19);
    hgenY.reseed(19);

    using IntPoint = Point<Point<int, int>, int>;
    using Keepout = decltype(std::declval<IntPoint>().enlarge_with(1));

    std::vector<IntPoint> terminals;
    for (int i = 0; i < 7; ++i) {
        terminals.emplace_back(Point{Point{static_cast<int>(hgenX.pop()), (i % 4) * scale_z},
                                     static_cast<int>(hgenY.pop())});
    }
    IntPoint source(Point{Point{static_cast<int>(hgenX.pop()), 0}, static_cast<int>(hgenY.pop())});
    std::vector<Keepout> keepouts = {
        {{Interval<int>(1600, 1900), Interval<int>(-1000, 1000)}, Interval<int>(1000, 1500)},
        {{Interval<int>(500, 800), Interval<int>(-1000, 1000)}, Interval<int>(600, 900)},
    };

    GlobalRouter<IntPoint> router(source, terminals, keepouts);
    router.route_with_constraints(1.0);

    std::string svg_output
        = visualize_routing_tree3d_svg(router.get_tree(), keepouts, scale_z, 1000, 1000);
    std::cout << svg_output;

    save_routing_tree3d_svg(router.get_tree(), keepouts, scale_z,
                            "example_route3d_with_constraint_and_keepouts.svg");
}
