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

// Helper function to generate a set of 3D points and keepouts for testing
auto generate_3d_points_and_keepouts(size_t num_terminals, unsigned int seed) -> std::tuple<
    Point<Point<int, int>, int>, std::vector<Point<Point<int, int>, int>>,
    std::vector<decltype(std::declval<Point<Point<int, int>, int>>().enlarge_with(1))>> {
    constexpr int scale_z = 100;

    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    hgenX.reseed(seed);
    hgenY.reseed(seed);

    using IntPoint3d = Point<Point<int, int>, int>;
    using Keepout3d = decltype(std::declval<IntPoint3d>().enlarge_with(1));

    std::vector<IntPoint3d> terminals;
    terminals.reserve(num_terminals);
    for (size_t i = 0; i < num_terminals; ++i) {
        terminals.emplace_back(
            Point{Point{static_cast<int>(hgenX.pop()), static_cast<int>(i % 4) * scale_z},
                  static_cast<int>(hgenY.pop())});
    }
    IntPoint3d source(
        Point{Point{static_cast<int>(hgenX.pop()), 0}, static_cast<int>(hgenY.pop())});

    std::vector<Keepout3d> keepouts = {
        {Point{Interval<int>(1600, 1900), Interval<int>(-1000, 1000)}, Interval<int>(1000, 1500)},
        {Point{Interval<int>(500, 800), Interval<int>(-1000, 1000)}, Interval<int>(600, 900)},
    };

    return {source, terminals, keepouts};
}

TEST_SUITE("RoutingAlgorithms3dWithKeepouts") {
    TEST_CASE("Test routing algorithms in 3D with keepouts") {
        const size_t num_terminals = 7;
        const unsigned int seed = 19;
        const double high_stress = 1.0;
        const int scale_z = 100;
        const unsigned int width = 1000;
        const unsigned int height = 1000;

        auto [source, terminals, keepouts] = generate_3d_points_and_keepouts(num_terminals, seed);

        SUBCASE("route_with_steiners") {
            GlobalRouter<Point<Point<int, int>, int>> router(source, terminals, keepouts);
            router.route_with_steiners();
            save_routing_tree3d_svg(router.get_tree(), keepouts, scale_z,
                                    "example_route3d_with_steiner_and_keepouts.svg", width, height);
        }

        SUBCASE("route_with_constraints") {
            GlobalRouter<Point<Point<int, int>, int>> router(source, terminals, keepouts);
            router.route_with_constraints(high_stress);
            save_routing_tree3d_svg(router.get_tree(), keepouts, scale_z,
                                    "example_route3d_with_constraint_and_keepouts.svg", width,
                                    height);
        }
    }
}
