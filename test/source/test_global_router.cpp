#include <doctest/doctest.h>
#include <fmt/core.h>

#include <iostream>         // for operator<<
#include <ldsgen/ilds.hpp>  // for VdCorput
#include <recti/global_router.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <vector>

using namespace recti;

TEST_CASE("Test route_with_steiner") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(19);
    hgenY.reseed(19);

    std::vector<IntPoint> terminals;
    for (int i = 0; i < 5; ++i) {
        terminals.emplace_back(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop()));
    }
    IntPoint source(static_cast<int>(hgenX.pop()), static_cast<int>(hgenX.pop()));

    GlobalRouter router(source, terminals);
    router.route_with_steiners();

    std::string svg_output = visualize_routing_tree_svg(router.get_tree(), 1000, 1000);
    std::cout << svg_output;

    save_routing_tree_svg(router.get_tree(), "example_route_with_steiner.svg");
}

TEST_CASE("Test route_with_constraints") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(19);
    hgenY.reseed(19);

    std::vector<IntPoint> terminals;
    for (int i = 0; i < 5; ++i) {
        terminals.emplace_back(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop()));
    }
    IntPoint source(static_cast<int>(hgenX.pop()), static_cast<int>(hgenX.pop()));

    GlobalRouter router(source, terminals);
    router.route_with_constraints(1.1);

    std::string svg_output = visualize_routing_tree_svg(router.get_tree(), 1000, 1000);
    std::cout << svg_output;

    save_routing_tree_svg(router.get_tree(), "example_route_with_constraint.svg");
}