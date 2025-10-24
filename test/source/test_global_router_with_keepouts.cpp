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

// Helper function to generate a set of points for testing
auto generate_points_and_keepouts(size_t num_terminals, unsigned int seed)
    -> std::tuple<Point<int, int>, std::vector<Point<int, int>>,
                  std::vector<decltype(std::declval<Point<int, int>>().enlarge_with(1))>> {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(seed);
    hgenY.reseed(seed);

    using IntPoint = Point<int, int>;
    using Keepout = decltype(std::declval<IntPoint>().enlarge_with(1));

    std::vector<IntPoint> terminals;
    terminals.reserve(num_terminals);
    for (size_t i = 0; i < num_terminals; ++i) {
        terminals.emplace_back(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop()));
    }
    IntPoint source(static_cast<int>(hgenX.pop()), static_cast<int>(hgenX.pop()));

    std::vector<Keepout> keepouts = {
        {Interval<int>(1600, 1900), Interval<int>(1000, 1500)},
        {Interval<int>(500, 800), Interval<int>(600, 900)},
    };

    return {source, terminals, keepouts};
}

TEST_SUITE("RoutingAlgorithmsWithKeepouts") {
    TEST_CASE("Test routing algorithms with keepouts") {
        const size_t num_terminals = 7;
        const unsigned int seed = 19;
        const double high_stress = 1.1;
        const unsigned int width = 1000;
        const unsigned int height = 1000;

        auto [source, terminals, keepouts] = generate_points_and_keepouts(num_terminals, seed);

        SUBCASE("route_with_steiners") {
            GlobalRouter<Point<int, int>> router(source, terminals, keepouts);
            router.route_with_steiners();
            save_routing_tree_svg(router.get_tree(), keepouts,
                                  "example_route_with_steiner_and_keepouts.svg", width, height);
        }

        SUBCASE("route_with_constraints") {
            GlobalRouter<Point<int, int>> router(source, terminals, keepouts);
            router.route_with_constraints(high_stress);
            save_routing_tree_svg(router.get_tree(), keepouts,
                                  "example_route_with_constraint_and_keepouts.svg", width,
                                  height);
        }
    }
}
