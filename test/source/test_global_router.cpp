#include <doctest/doctest.h>
#include <fmt/core.h>

#include <iostream>         // for operator<<
#include <ldsgen/ilds.hpp>  // for VdCorput
#include <recti/global_router.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <string>
#include <vector>

#include "TestGlobalRouter.h"

namespace {
using namespace recti;
// Helper function to generate a set of points for testing
auto generate_points(size_t num_terminals, unsigned int seed)
    -> std::pair<Point<int, int>, std::vector<Point<int, int>>> {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(seed);
    hgenY.reseed(seed);

    using IntPoint = Point<int, int>;

    std::vector<IntPoint> terminals;
    terminals.reserve(num_terminals);
    for (size_t i = 0; i < num_terminals; ++i) {
        terminals.emplace_back(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop()));
    }
    IntPoint source(static_cast<int>(hgenX.pop()), static_cast<int>(hgenX.pop()));

    return {source, terminals};
}
} // namespace

TEST_SUITE("RoutingAlgorithms") {
    TEST_CASE("Test routing algorithms") {
        const size_t num_terminals = 7;
        const unsigned int seed = 19;
        const double high_stress = 1.0;
        const unsigned int width = 1000;
        const unsigned int height = 1000;

        auto [source, terminals] = generate_points(num_terminals, seed);

        SUBCASE("route_with_steiners") {
            GlobalRouter router(source, terminals);
            router.route_with_steiners();
            save_routing_tree_svg(router.get_tree(), std::nullopt,
                                  "example_route_with_steiner.svg", width, height);
        }

        SUBCASE("route_with_constraints") {
            GlobalRouter router(source, terminals);
            router.route_with_constraints(high_stress);
            save_routing_tree_svg(router.get_tree(), std::nullopt,
                                  "example_route_with_constraint.svg", width, height);
        }
    }
}

