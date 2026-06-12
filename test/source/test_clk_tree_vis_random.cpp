#include <doctest/doctest.h>
#include <fmt/core.h>

#include <filesystem>
#include <lds/ilds.hpp>
#include <ldsgen/ilds.hpp>
#include <memory>
#include <recti/visualize_clock_tree.hpp>
#include <string>
#include <vector>

using namespace recti;

static auto generate_random_sinks(size_t num_sinks, unsigned int seed) -> std::vector<Sink> {
    ilds::VdCorput<3> hgenX(7);
    ilds::VdCorput<2> hgenY(11);

    hgenX.reseed(seed);
    hgenY.reseed(seed);

    std::vector<Sink> sinks;
    sinks.reserve(num_sinks);
    for (size_t i = 0; i < num_sinks; ++i) {
        sinks.emplace_back(fmt::format("s{}", i), Point<int>(static_cast<int>(hgenX.pop()),
                                                             static_cast<int>(hgenY.pop())));
    }
    return sinks;
}

TEST_CASE("Random sink stress test") {
    SUBCASE("Complete Workflow Example") {
        const size_t num_sinks = 100;
        const unsigned int seed = 1;

        std::vector<Sink> sinks = generate_random_sinks(num_sinks, seed);

        auto calculator = std::make_unique<LinearDelayCalculator>(0.8, 0.15);
        DMEAlgorithm dme(sinks, std::move(calculator));
        NodeIdx root = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(root);

        ClockTreeVisualizer basic_viz;
        basic_viz.visualize_tree(dme.get_tree(), root, sinks, "basic_clock_tree.svg");

        create_interactive_svg(dme.get_tree(), root, sinks, &analysis,
                               "interactive_clock_tree.svg");

        CHECK(std::filesystem::exists("basic_clock_tree.svg"));
        CHECK(std::filesystem::exists("interactive_clock_tree.svg"));

        MESSAGE("Example visualization workflow completed successfully");
    }
}
