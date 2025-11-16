#include <doctest/doctest.h>
#include <fmt/core.h>

#include <iostream>
#include <ldsgen/ilds.hpp>
#include <recti/global_router.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <string>
#include <vector>
#include <random>

namespace {
using namespace recti;

// Helper function to generate a set of points for testing
auto generate_random_points(size_t num_terminals, int max_coord)
    -> std::pair<Point<int, int>, std::vector<Point<int, int>>> {
    
    std::mt19937 gen(12345);  // for reproducible results
    std::uniform_int_distribution<> distrib_coord(0, max_coord);

    using IntPoint = Point<int, int>;

    std::vector<IntPoint> terminals;
    terminals.reserve(num_terminals);
    for (size_t i = 0; i < num_terminals; ++i) {
        terminals.emplace_back(distrib_coord(gen), distrib_coord(gen));
    }
    IntPoint source(distrib_coord(gen), distrib_coord(gen));

    return {source, terminals};
}
} // namespace

TEST_SUITE("GlobalRouter Stress Tests") {
    TEST_CASE("Large number of terminals") {
        const size_t num_terminals = 200;
        const int max_coord = 10000;
        const double high_stress = 1.0;

        auto [source, terminals] = generate_random_points(num_terminals, max_coord);

        SUBCASE("route_with_steiners") {
            GlobalRouter router(source, terminals);
            router.route_with_steiners();
            auto& tree = router.get_tree();
            CHECK(tree.nodes.size() >= num_terminals + 1);
        }

        SUBCASE("route_with_constraints") {
            GlobalRouter router(source, terminals);
            router.route_with_constraints(high_stress);
            auto& tree = router.get_tree();
            CHECK(tree.nodes.size() >= num_terminals + 1);
        }
    }

    TEST_CASE("Varying stress factor") {
        const size_t num_terminals = 50;
        const int max_coord = 1000;

        auto [source, terminals] = generate_random_points(num_terminals, max_coord);

        SUBCASE("Low stress") {
            GlobalRouter router(source, terminals);
            router.route_with_constraints(0.1);
            auto& tree = router.get_tree();
            CHECK(tree.nodes.size() >= num_terminals + 1);
        }

        SUBCASE("Medium stress") {
            GlobalRouter router(source, terminals);
            router.route_with_constraints(0.5);
            auto& tree = router.get_tree();
            CHECK(tree.nodes.size() >= num_terminals + 1);
        }

        SUBCASE("High stress") {
            GlobalRouter router(source, terminals);
            router.route_with_constraints(1.0);
            auto& tree = router.get_tree();
            CHECK(tree.nodes.size() >= num_terminals + 1);
        }
    }
}