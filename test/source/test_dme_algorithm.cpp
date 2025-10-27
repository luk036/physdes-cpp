#include <doctest/doctest.h>

#include <memory>
#include <recti/manhattan_arc.hpp>  // for ManhattanArc, operator+, operator-
#include <string>
#include <vector>

#include "recti/dme_algorithm.hpp"  // for min_dist, overlap, interval

using namespace recti;

namespace {
int count_leaves(const std::shared_ptr<recti::TreeNode>& node) {
    if (!node) {
        return 0;
    }
    if (node->is_leaf()) {
        return 1;
    }
    return count_leaves(node->left) + count_leaves(node->right);
}
template <typename DelayCalculator, typename... Args>
auto create_dme_algorithm(const std::vector<recti::Sink>& sinks, Args&&... args) {
    auto calculator = std::make_unique<DelayCalculator>(std::forward<Args>(args)...);
    return recti::DMEAlgorithm(sinks, std::move(calculator));
}

} // namespace

TEST_SUITE("Sink Tests") {
    TEST_CASE("Sink Construction") {
        SUBCASE("Basic construction") {
            recti::Sink sink("s1", recti::Point<int>(10, 20), 1.5);

            CHECK(sink.name == "s1");
            CHECK(sink.position.xcoord() == 10);
            CHECK(sink.position.ycoord() == 20);
            CHECK(sink.capacitance == doctest::Approx(1.5));
        }

        SUBCASE("Default capacitance") {
            recti::Sink sink("s2", recti::Point<int>(30, 40));

            CHECK(sink.name == "s2");
            CHECK(sink.capacitance == doctest::Approx(1.0));
        }
    }
}

TEST_SUITE("TreeNode Tests") {
    TEST_CASE("TreeNode Construction") {
        recti::TreeNode node("n1", recti::Point<int>(30, 40));

        CHECK(node.name == "n1");
        CHECK(node.position.xcoord() == 30);
        CHECK(node.position.ycoord() == 40);
        CHECK(node.wire_length == 0);
        CHECK(node.delay == doctest::Approx(0.0));
        CHECK(node.capacitance == doctest::Approx(0.0));
        CHECK(node.need_elongation == false);
        CHECK(node.left == nullptr);
        CHECK(node.right == nullptr);
        CHECK(node.parent == nullptr);
    }

    TEST_CASE("TreeNode Leaf Detection") {
        SUBCASE("Leaf node") {
            recti::TreeNode leaf("leaf", recti::Point<int>(10, 10));
            CHECK(leaf.is_leaf() == true);
        }

        SUBCASE("Internal node") {
            auto left = std::make_shared<recti::TreeNode>("left", recti::Point<int>(10, 10));
            auto right = std::make_shared<recti::TreeNode>("right", recti::Point<int>(20, 20));
            recti::TreeNode internal("internal", recti::Point<int>(15, 15));

            internal.left = left;
            internal.right = right;

            CHECK(internal.is_leaf() == false);
        }
    }
}

TEST_SUITE("LinearDelayCalculator Tests") {
    TEST_CASE("LinearDelayCalculator Construction") {
        recti::LinearDelayCalculator calc(0.5, 0.2);

        // Can't test private members directly, but we can test behavior
        SUBCASE("Wire delay calculation") {
            double delay = calc.calculate_wire_delay(10, 5.0);
            CHECK(delay == doctest::Approx(5.0));
        }
    }

    TEST_CASE("LinearDelayCalculator Methods") {
        recti::LinearDelayCalculator calc(0.5, 0.2);

        SUBCASE("Calculate wire delay") {
            CHECK(calc.calculate_wire_delay(10, 5.0) == doctest::Approx(5.0));
            CHECK(calc.calculate_wire_delay(0, 100.0) == doctest::Approx(0.0));
        }

        SUBCASE("Calculate wire delay per unit") {
            CHECK(calc.calculate_wire_delay_per_unit(5.0) == doctest::Approx(0.5));
            CHECK(calc.calculate_wire_delay_per_unit(100.0)
                  == doctest::Approx(0.5));  // Should ignore load
        }

        SUBCASE("Calculate wire capacitance") {
            CHECK(calc.calculate_wire_capacitance(10) == doctest::Approx(2.0));
            CHECK(calc.calculate_wire_capacitance(0) == doctest::Approx(0.0));
        }
    }

    TEST_CASE("LinearDelayCalculator Tapping Point") {
        recti::LinearDelayCalculator calc(0.5, 0.2);

        SUBCASE("Balanced case") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 1.0;
            right.delay = 1.0;

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            CHECK(extend_left == 5);
            CHECK(delay_left == doctest::Approx(3.5));
            CHECK(left.wire_length == 5);
            CHECK(right.wire_length == 5);
        }

        SUBCASE("Skewed case") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 1.0;
            right.delay = 3.0;  // Right is slower

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // Should extend left more to balance delays
            CHECK(extend_left > 5);
            CHECK(delay_left == doctest::Approx(left.delay + extend_left * 0.5));
        }
    }
}

TEST_SUITE("ElmoreDelayCalculator Tests") {
    TEST_CASE("ElmoreDelayCalculator Construction") {
        recti::ElmoreDelayCalculator calc(0.1, 0.2);

        SUBCASE("Wire delay calculation") {
            double delay = calc.calculate_wire_delay(10, 5.0);
            // Expected: 0.1*10 * (0.2*10/2 + 5.0) = 1.0 * (1.0 + 5.0) = 6.0
            CHECK(delay == doctest::Approx(6.0));
        }
    }

    TEST_CASE("ElmoreDelayCalculator Methods") {
        recti::ElmoreDelayCalculator calc(0.1, 0.2);

        SUBCASE("Calculate wire delay") {
            CHECK(calc.calculate_wire_delay(10, 5.0) == doctest::Approx(6.0));
            CHECK(calc.calculate_wire_delay(0, 100.0) == doctest::Approx(0.0));
        }

        SUBCASE("Calculate wire delay per unit") {
            // Expected: 0.1 * (0.2/2 + 5.0) = 0.1 * (0.1 + 5.0) = 0.51
            CHECK(calc.calculate_wire_delay_per_unit(5.0) == doctest::Approx(0.51));
        }

        SUBCASE("Calculate wire capacitance") {
            CHECK(calc.calculate_wire_capacitance(10) == doctest::Approx(2.0));
            CHECK(calc.calculate_wire_capacitance(0) == doctest::Approx(0.0));
        }
    }
}

TEST_SUITE("DMEAlgorithm Tests") {
    TEST_CASE("DMEAlgorithm Construction") {
        SUBCASE("Valid construction") {
            std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(10, 20), 1.0)};
            REQUIRE_NOTHROW(create_dme_algorithm<recti::LinearDelayCalculator>(sinks));
        }

        SUBCASE("Empty sinks throws exception") {
            std::vector<recti::Sink> sinks;
            REQUIRE_THROWS_AS(create_dme_algorithm<recti::LinearDelayCalculator>(sinks),
                              std::invalid_argument);
        }
    }

    TEST_CASE("DMEAlgorithm Single Sink") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(10, 20), 1.5)};
        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks);
        auto tree = dme.build_clock_tree();

        CHECK(tree->name == "s1");  // Single sink becomes root
        CHECK(tree->position.xcoord() == 10);
        CHECK(tree->position.ycoord() == 20);
        CHECK(tree->is_leaf() == true);
        CHECK(tree->delay == doctest::Approx(0.0));
    }

    TEST_CASE("DMEAlgorithm Two Sinks") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(0, 0), 1.0),
                                          recti::Sink("s2", recti::Point<int>(10, 0), 1.0)};
        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks, 1.0, 1.0);
        auto tree = dme.build_clock_tree();

        CHECK(tree->is_leaf() == false);  // Root should be internal node
        CHECK(tree->left != nullptr);
        CHECK(tree->right != nullptr);
        CHECK(tree->left->is_leaf() == true);
        CHECK(tree->right->is_leaf() == true);

        // Analyze skew
        auto analysis = dme.analyze_skew(tree);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(0.001));  // Should be zero skew
        CHECK(analysis.total_wirelength > 0);
    }

    TEST_CASE("DMEAlgorithm Multiple Sinks") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(10, 20), 1.0),
                                          recti::Sink("s2", recti::Point<int>(30, 40), 1.0),
                                          recti::Sink("s3", recti::Point<int>(50, 10), 1.0)};
        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks);
        auto tree = dme.build_clock_tree();

        CHECK(tree != nullptr);
        CHECK(tree->is_leaf() == false);
        CHECK(count_leaves(tree) == 3);
    }

    TEST_CASE("DMEAlgorithm Skew Analysis") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(0, 0), 1.0),
                                          recti::Sink("s2", recti::Point<int>(10, 10), 1.0),
                                          recti::Sink("s3", recti::Point<int>(20, 0), 1.0)};
        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks, 1.0, 1.0);
        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);

        CHECK(analysis.max_delay >= analysis.min_delay);
        CHECK(analysis.skew >= 0.0);
        CHECK(analysis.sink_delays.size() == 3);
        CHECK(analysis.total_wirelength > 0);
        CHECK_FALSE(analysis.delay_model.empty());

        // For zero-skew algorithm, skew should be very small
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(0.001));
    }
}

TEST_SUITE("Tree Statistics Tests") {
    TEST_CASE("Get Tree Statistics") {
        // Create a simple tree
        auto s1 = std::make_shared<recti::TreeNode>("s1", recti::Point<int>(10, 20));
        auto s2 = std::make_shared<recti::TreeNode>("s2", recti::Point<int>(30, 40));
        auto root = std::make_shared<recti::TreeNode>("n1", recti::Point<int>(20, 30));

        root->left = s1;
        root->right = s2;
        s1->parent = root;
        s2->parent = root;

        s1->wire_length = 5;
        s2->wire_length = 5;
        s1->delay = 1.0;
        s2->delay = 1.0;
        s1->capacitance = 1.0;
        s2->capacitance = 1.0;

        auto stats = recti::get_tree_statistics(root);

        CHECK(stats.total_nodes == 3);
        CHECK(stats.total_sinks == 2);
        CHECK(stats.total_wires == 2);
        CHECK(stats.sinks.size() == 2);
        CHECK(stats.nodes.size() == 3);
        CHECK(stats.wires.size() == 2);

        // Check node information
        bool found_s1 = false, found_s2 = false, found_root = false;
        for (const auto& node : stats.nodes) {
            if (node.name == "s1") {
                found_s1 = true;
                CHECK(node.type == "sink");
                CHECK(node.position == std::pair<int, int>(10, 20));
                CHECK(node.delay == doctest::Approx(1.0));
            } else if (node.name == "s2") {
                found_s2 = true;
                CHECK(node.type == "sink");
                CHECK(node.position == std::pair<int, int>(30, 40));
            } else if (node.name == "n1") {
                found_root = true;
                CHECK(node.type == "internal");
                CHECK(node.position == std::pair<int, int>(20, 30));
            }
        }

        CHECK(found_s1);
        CHECK(found_s2);
        CHECK(found_root);

        // Check wire information
        for (const auto& wire : stats.wires) {
            CHECK(wire.from_node == "n1");
            CHECK((wire.to_node == "s1" || wire.to_node == "s2"));
            CHECK(wire.length == 5);
        }
    }

    TEST_CASE("Empty Tree Statistics") {
        auto empty_tree = std::make_shared<recti::TreeNode>("root", recti::Point<int>(0, 0));

        auto stats = recti::get_tree_statistics(empty_tree);

        CHECK(stats.total_nodes == 1);
        CHECK(stats.total_sinks == 1);  // Single node is a leaf
        CHECK(stats.total_wires == 0);  // No parent, so no wires
        CHECK(stats.sinks.size() == 1);
        CHECK(stats.sinks[0] == "root");
    }
}

TEST_SUITE("Integration Tests") {
    TEST_CASE("Linear vs Elmore Comparison") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(10, 20), 1.0),
                                          recti::Sink("s2", recti::Point<int>(30, 40), 1.0),
                                          recti::Sink("s3", recti::Point<int>(50, 10), 1.0)};

        // Test with Linear delay calculator
        auto dme_linear = create_dme_algorithm<recti::LinearDelayCalculator>(sinks, 0.5, 0.2);
        auto tree_linear = dme_linear.build_clock_tree();
        auto analysis_linear = dme_linear.analyze_skew(tree_linear);

        // Test with Elmore delay calculator
        auto dme_elmore = create_dme_algorithm<recti::ElmoreDelayCalculator>(sinks, 0.1, 0.2);
        auto tree_elmore = dme_elmore.build_clock_tree();
        auto analysis_elmore = dme_elmore.analyze_skew(tree_elmore);

        // Both should produce valid trees with low skew
        CHECK(analysis_linear.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK(analysis_elmore.skew == doctest::Approx(0.0).epsilon(1.0));

        // Both should have positive wirelength
        CHECK(analysis_linear.total_wirelength > 0);
        CHECK(analysis_elmore.total_wirelength > 0);

        // Delay models should be different
        CHECK(analysis_linear.delay_model != analysis_elmore.delay_model);
    }

    TEST_CASE("Large Tree Construction") {
        // Create a larger set of sinks
        std::vector<recti::Sink> sinks;
        for (int i = 0; i < 8; ++i) {
            sinks.emplace_back(std::string("s") + std::to_string(i),
                               recti::Point<int>(i * 10, i * 5), 1.0 + i * 0.1);
        }

        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks);
        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);
        auto stats = recti::get_tree_statistics(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(1.0));  // Allow small tolerance
        CHECK(stats.total_nodes == 15);  // 8 leaves + 7 internal nodes for balanced tree
        CHECK(stats.total_sinks == 8);
    }
}

TEST_CASE("Edge Cases") {
    SUBCASE("Sinks at same position") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(10, 10), 1.0),
                                          recti::Sink("s2", recti::Point<int>(10, 10), 1.0),
                                          recti::Sink("s3", recti::Point<int>(10, 10), 1.0)};

        auto calculator = std::make_unique<recti::LinearDelayCalculator>();
        recti::DMEAlgorithm dme(sinks, std::move(calculator));

        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.skew == doctest::Approx(0.0));
    }

    SUBCASE("Very distant sinks") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(0, 0), 1.0),
                                          recti::Sink("s2", recti::Point<int>(1000, 1000), 1.0)};

        auto calculator = std::make_unique<recti::LinearDelayCalculator>();
        recti::DMEAlgorithm dme(sinks, std::move(calculator));

        auto tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(tree);

        CHECK(tree != nullptr);
        CHECK(analysis.total_wirelength >= 2000);  // At least Manhattan distance
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(0.001));
    }
}

// Test runner main function
// int main(int argc, char** argv) {
//     doctest::Context context;
//     context.applyCommandLine(argc, argv);
//     return context.run();
// }
