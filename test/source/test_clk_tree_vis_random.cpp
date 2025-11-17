#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST...

#include <recti/halton_int.hpp>            // for recti
#include <recti/visualize_clock_tree.hpp>  // for ManhattanArc, operator+, operator-
#include <ldsgen/ilds.hpp>  // for VdCorput


#include <doctest/doctest.h>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>

using namespace recti;

// Helper function to generate a set of points for testing
static auto generate_random_sinks(size_t num_sinks, unsigned int seed)
    -> std::vector<recti::Sink> {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(seed);
    hgenY.reseed(seed);

    std::vector<recti::Sink> sinks;
    sinks.reserve(num_sinks);
    for (size_t i = 0; i < num_sinks; ++i) {
        sinks.emplace_back(fmt::format("s{}", i),
                           recti::Point<int>(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop())));
    }

    return sinks;
}

// Random sinks stress test
TEST_CASE("Random sink stress test") {
    SUBCASE("Complete Workflow Example") {
        const size_t num_sinks = 100;
        const unsigned int seed = 1;

        // Step 1: Create clock sinks
        std::vector<recti::Sink> sinks = generate_random_sinks(num_sinks, seed);
        
        // Step 2: Build clock tree using DME algorithm
        auto calculator = std::make_unique<recti::LinearDelayCalculator>(0.8, 0.15);
        recti::DMEAlgorithm dme(sinks, std::move(calculator));
        auto clock_tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(clock_tree);
        
        // Step 3: Create basic visualization
        recti::ClockTreeVisualizer basic_viz;
        basic_viz.visualize_tree(clock_tree, sinks, "basic_clock_tree.svg");
        
        // Step 4: Create interactive visualization with analysis
        recti::create_interactive_svg(clock_tree, sinks, &analysis, "interactive_clock_tree.svg");
        
        // Step 5: Verify files were created
        CHECK(std::filesystem::exists("basic_clock_tree.svg"));
        CHECK(std::filesystem::exists("interactive_clock_tree.svg"));
        
        // Step 6: Cleanup
        // std::filesystem::remove("basic_clock_tree.svg");
        // std::filesystem::remove("interactive_clock_tree.svg");
        
        MESSAGE("Example visualization workflow completed successfully");
    }
}
