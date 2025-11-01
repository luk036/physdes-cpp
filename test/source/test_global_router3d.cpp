#include <doctest/doctest.h>
#include <fmt/core.h>

#include <cstdlib>          // for rand()
#include <iostream>         // for operator<<
#include <ldsgen/ilds.hpp>  // for VdCorput
#include <recti/global_router.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <string>
#include <vector>

#include "TestGlobalRouter.h"

using namespace recti;

namespace {
using namespace recti;
// Helper function to generate a set of 3D points for testing
auto generate_3d_points(size_t num_terminals, unsigned int seed)
    -> std::pair<Point<Point<int, int>, int>, std::vector<Point<Point<int, int>, int>>> {
    constexpr int scale_z = 100;

    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);
    hgenX.reseed(seed);
    hgenY.reseed(seed);

    using IntPoint3d = Point<Point<int, int>, int>;

    std::vector<IntPoint3d> terminals;
    terminals.reserve(num_terminals);
    for (size_t i = 0; i < num_terminals; ++i) {
        terminals.emplace_back(
            Point{Point{static_cast<int>(hgenX.pop()), (static_cast<int>(i) % 4) * scale_z},
                  static_cast<int>(hgenY.pop())});
    }
    IntPoint3d source(
        Point{Point{static_cast<int>(hgenX.pop()), 0}, static_cast<int>(hgenY.pop())});

    return {source, terminals};
}
} // namespace

TEST_SUITE("RoutingAlgorithms3d") {
    TEST_CASE("Test routing algorithms in 3D") {
        const size_t num_terminals = 5;
        const unsigned int seed = 19;
        const double high_stress = 1.0;
        const int scale_z = 100;
        const unsigned int width = 1000;
        const unsigned int height = 1000;

        auto [source, terminals] = generate_3d_points(num_terminals, seed);

        SUBCASE("route_with_steiners") {
            GlobalRouter router(source, terminals);
            router.route_with_steiners();
            save_routing_tree3d_svg(router.get_tree(), std::nullopt, scale_z,
                                    "example_route3d_with_steiner.svg", width, height);
        }

        SUBCASE("route_with_constraints") {
            GlobalRouter router(source, terminals);
            router.route_with_constraints(high_stress);
            save_routing_tree3d_svg(router.get_tree(), std::nullopt, scale_z,
                                    "example_route3d_with_constraint.svg", width, height);
        }
    }
}

