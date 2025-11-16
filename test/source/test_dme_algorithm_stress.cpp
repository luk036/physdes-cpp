#include <doctest/doctest.h>
#include <fmt/core.h>

#include <memory>
#include <random>
#include <recti/manhattan_arc.hpp>
#include <string>
#include <vector>

#include "recti/dme_algorithm.hpp"

using namespace recti;

namespace {
template <typename DelayCalculator, typename... Args>
auto create_dme_algorithm(const std::vector<recti::Sink>& sinks, Args&&... args) {
    auto calculator = std::make_unique<DelayCalculator>(std::forward<Args>(args)...);
    return recti::DMEAlgorithm(sinks, std::move(calculator));
}

std::vector<recti::Sink> generate_random_sinks(int num_sinks, int max_coord,
                                               double max_cap) {
    std::vector<recti::Sink> sinks;
    std::mt19937 gen(12345);  // for reproducible results
    std::uniform_int_distribution<> distrib_coord(0, max_coord);
    std::uniform_real_distribution<> distrib_cap(0.1, max_cap);

    for (int i = 0; i < num_sinks; ++i) {
        sinks.emplace_back(fmt::format("s{}", i),
                           recti::Point<int>(distrib_coord(gen), distrib_coord(gen)),
                           distrib_cap(gen));
    }
    return sinks;
}

}  // namespace

TEST_SUITE("DMEAlgorithm Stress Tests") {
    TEST_CASE("Large number of random sinks") {
        const int num_sinks = 256;
        const int max_coord = 10000;
        const double max_cap = 5.0;

        auto sinks = generate_random_sinks(num_sinks, max_coord, max_cap);
        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks);

        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);
        auto stats = recti::get_tree_statistics(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK(stats.total_sinks == num_sinks);
        CHECK(stats.total_nodes == 2 * num_sinks - 1);
    }

    TEST_CASE("Sinks in a grid pattern") {
        std::vector<recti::Sink> sinks;
        const int grid_size = 10;
        for (int i = 0; i < grid_size; ++i) {
            for (int j = 0; j < grid_size; ++j) {
                sinks.emplace_back(fmt::format("s{}", i * grid_size + j),
                                   recti::Point<int>(i * 100, j * 100), 1.0);
            }
        }

        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks);
        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);
        auto stats = recti::get_tree_statistics(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK(stats.total_sinks == grid_size * grid_size);
        CHECK(stats.total_nodes == 2 * grid_size * grid_size - 1);
    }

    TEST_CASE("Sinks clustered in a small area") {
        const int num_sinks = 100;
        const int max_coord = 100;  // Small area
        const double max_cap = 2.0;

        auto sinks = generate_random_sinks(num_sinks, max_coord, max_cap);
        auto dme = create_dme_algorithm<recti::ElmoreDelayCalculator>(sinks, 0.1, 0.2);

        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);
        auto stats = recti::get_tree_statistics(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK(stats.total_sinks == num_sinks);
        CHECK(stats.total_nodes == 2 * num_sinks - 1);
    }

    TEST_CASE("Sinks along a horizontal line") {
        std::vector<recti::Sink> sinks;
        const int num_sinks = 50;
        for (int i = 0; i < num_sinks; ++i) {
            sinks.emplace_back(fmt::format("s{}", i),
                               recti::Point<int>(i * 100, 500), 1.0);
        }

        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks);
        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);
        auto stats = recti::get_tree_statistics(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK(stats.total_sinks == num_sinks);
        CHECK(stats.total_nodes == 2 * num_sinks - 1);
    }

    TEST_CASE("Sinks along a vertical line") {
        std::vector<recti::Sink> sinks;
        const int num_sinks = 50;
        for (int i = 0; i < num_sinks; ++i) {
            sinks.emplace_back(fmt::format("s{}", i),
                               recti::Point<int>(500, i * 100), 1.0);
        }

        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks);
        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);
        auto stats = recti::get_tree_statistics(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK(stats.total_sinks == num_sinks);
        CHECK(stats.total_nodes == 2 * num_sinks - 1);
    }
}
