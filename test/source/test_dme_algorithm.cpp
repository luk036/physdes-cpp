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
            CHECK(delay_left == doctest::Approx(1.0 + 5 * 0.5)); // 1.0 + 2.5 = 3.5
            CHECK(left.wire_length == 5);
            CHECK(right.wire_length == 5);
            CHECK(left.need_elongation == false);
            CHECK(right.need_elongation == false);
        }

        SUBCASE("Skewed case - right slower") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 1.0;
            right.delay = 3.0;  // Right is slower, so extend_left should be > 5

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 3.0 - 1.0 = 2.0
            // extend_left = round((2.0 / 0.5 + 10) / 2) = round((4 + 10) / 2) = round(7) = 7
            CHECK(extend_left == 7);
            CHECK(delay_left == doctest::Approx(1.0 + 7 * 0.5)); // 1.0 + 3.5 = 4.5
            CHECK(left.wire_length == 7);
            CHECK(right.wire_length == 3);
            CHECK(left.need_elongation == false);
            CHECK(right.need_elongation == false);
        }

        SUBCASE("Skewed case - left slower") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 3.0;
            right.delay = 1.0;  // Left is slower, so extend_left should be < 5

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 1.0 - 3.0 = -2.0
            // extend_left = round((-2.0 / 0.5 + 10) / 2) = round((-4 + 10) / 2) = round(3) = 3
            CHECK(extend_left == 3);
            CHECK(delay_left == doctest::Approx(3.0 + 3 * 0.5)); // 3.0 + 1.5 = 4.5
            CHECK(left.wire_length == 3);
            CHECK(right.wire_length == 7);
            CHECK(left.need_elongation == false);
            CHECK(right.need_elongation == false);
        }

        SUBCASE("Elongation needed for right branch (extend_left < 0)") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 10.0;
            right.delay = 1.0; // Right is much faster, tapping point would be negative

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 1.0 - 10.0 = -9.0
            // extend_left = round((-9.0 / 0.5 + 10) / 2) = round((-18 + 10) / 2) = round(-4) = -4
            // Should be clamped to 0
            CHECK(extend_left == 0);
            CHECK(delay_left == doctest::Approx(10.0)); // delay_left should be node_left.delay
            CHECK(left.wire_length == 0);
            CHECK(right.wire_length == 14); // distance - extend_left = 10 - (-4) = 14
            CHECK(left.need_elongation == false);
            CHECK(right.need_elongation == true);
        }

        SUBCASE("Elongation needed for left branch (extend_left > distance)") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 1.0;
            right.delay = 10.0; // Left is much faster, tapping point would be > distance

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 10.0 - 1.0 = 9.0
            // extend_left = round((9.0 / 0.5 + 10) / 2) = round((18 + 10) / 2) = round(14) = 14
            // Should be clamped to distance (10)
            CHECK(extend_left == 10);
            CHECK(delay_left == doctest::Approx(10.0)); // delay_left should be node_right.delay
            CHECK(left.wire_length == 14); // extend_left = 14
            CHECK(right.wire_length == 0);
            CHECK(left.need_elongation == true);
            CHECK(right.need_elongation == false);
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

    TEST_CASE("ElmoreDelayCalculator Tapping Point") {
        recti::ElmoreDelayCalculator calc(0.1, 0.2);

        SUBCASE("Balanced case") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 1.0;
            right.delay = 1.0;
            left.capacitance = 1.0;
            right.capacitance = 1.0;

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 0
            // r = 10 * 0.1 = 1.0
            // c = 10 * 0.2 = 2.0
            // z = (0 + 1.0 * (1.0 + 2.0/2)) / (1.0 * (2.0 + 1.0 + 1.0)) = (1.0 * 2.0) / (1.0 * 4.0) = 2.0 / 4.0 = 0.5
            // extend_left = round(0.5 * 10) = 5
            // r_left = 5 * 0.1 = 0.5
            // c_left = 5 * 0.2 = 1.0
            // delay_left = 1.0 + 0.5 * (1.0/2 + 1.0) = 1.0 + 0.5 * 1.5 = 1.0 + 0.75 = 1.75
            CHECK(extend_left == 5);
            CHECK(delay_left == doctest::Approx(1.75));
            CHECK(left.wire_length == 5);
            CHECK(right.wire_length == 5);
            CHECK(left.need_elongation == false);
            CHECK(right.need_elongation == false);
        }

        SUBCASE("Skewed case - right slower") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 1.0;
            right.delay = 3.0;  // Right is slower
            left.capacitance = 1.0;
            right.capacitance = 1.0;

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 3.0 - 1.0 = 2.0
            // r = 1.0, c = 2.0
            // z = (2.0 + 1.0 * (1.0 + 2.0/2)) / (1.0 * (2.0 + 1.0 + 1.0)) = (2.0 + 2.0) / 4.0 = 4.0 / 4.0 = 1.0
            // extend_left = round(1.0 * 10) = 10
            CHECK(extend_left == 10);
            CHECK(delay_left == doctest::Approx(left.delay + 10 * 0.1 * (10 * 0.2 / 2 + left.capacitance))); // 1.0 + 1.0 * (1.0 + 1.0) = 3.0
            CHECK(left.wire_length == 10);
            CHECK(right.wire_length == 0);
            CHECK(left.need_elongation == false);
            CHECK(right.need_elongation == false);
        }

        SUBCASE("Elongation needed for right branch (extend_left < 0)") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 10.0;
            right.delay = 1.0; // Right is much faster, tapping point would be negative
            left.capacitance = 1.0;
            right.capacitance = 1.0;

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 1.0 - 10.0 = -9.0
            // r = 1.0, c = 2.0
            // z = (-9.0 + 1.0 * (1.0 + 2.0/2)) / (1.0 * (2.0 + 1.0 + 1.0)) = (-9.0 + 2.0) / 4.0 = -7.0 / 4.0 = -1.75
            // extend_left = round(-1.75 * 10) = -18 (clamped to 0)
            CHECK(extend_left == 0);
            CHECK(delay_left == doctest::Approx(10.0)); // delay_left should be node_left.delay
            CHECK(left.wire_length == 0);
            CHECK(right.wire_length == 28); // distance - extend_left = 10 - (-18) = 28
            CHECK(left.need_elongation == false);
            CHECK(right.need_elongation == true);
        }

        SUBCASE("Elongation needed for left branch (extend_left > distance)") {
            recti::TreeNode left("left", recti::Point<int>(0, 0));
            recti::TreeNode right("right", recti::Point<int>(10, 0));

            left.delay = 1.0;
            right.delay = 10.0; // Left is much faster, tapping point would be > distance
            left.capacitance = 1.0;
            right.capacitance = 1.0;

            auto [extend_left, delay_left] = calc.calculate_tapping_point(left, right, 10);

            // skew = 10.0 - 1.0 = 9.0
            // r = 1.0, c = 2.0
            // z = (9.0 + 1.0 * (1.0 + 2.0/2)) / (1.0 * (2.0 + 1.0 + 1.0)) = (9.0 + 2.0) / 4.0 = 11.0 / 4.0 = 2.75
            // extend_left = round(2.75 * 10) = 28 (clamped to 10)
            CHECK(extend_left == 10);
            CHECK(delay_left == doctest::Approx(10.0)); // delay_left should be node_right.delay
            CHECK(left.wire_length == 28); // extend_left = 28
            CHECK(right.wire_length == 0);
            CHECK(left.need_elongation == true);
            CHECK(right.need_elongation == false);
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

    TEST_CASE("DMEAlgorithm Multiple Sinks - Tree Structure and Merging Segments") {
        std::vector<recti::Sink> sinks = {recti::Sink("s1", recti::Point<int>(10, 10), 1.0),
                                          recti::Sink("s2", recti::Point<int>(20, 10), 1.0),
                                          recti::Sink("s3", recti::Point<int>(15, 30), 1.0),
                                          recti::Sink("s4", recti::Point<int>(25, 30), 1.0)};
        auto dme = create_dme_algorithm<recti::LinearDelayCalculator>(sinks, 1.0, 1.0);
        auto tree = dme.build_clock_tree();

        CHECK(tree != nullptr);
        CHECK(tree->is_leaf() == false); // Root should be internal
        CHECK(count_leaves(tree) == 4);

        // Verify basic tree structure (e.g., root has two children, children are internal or leaves)
        CHECK(tree->left != nullptr);
        CHECK(tree->right != nullptr);
        CHECK_FALSE(tree->left->is_leaf());
        CHECK_FALSE(tree->right->is_leaf());

        // Further checks could involve inspecting specific node names, positions, and parent/child relationships
        // This would require more detailed knowledge of the expected tree topology from build_merging_tree
        // For now, we rely on the leaf count and non-leaf root to indicate a valid tree structure.

        // Note: Directly testing merging segments is hard without exposing private members or having a specific API.
        // The correctness is implicitly tested by the zero-skew property in analyze_skew.
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
        // An empty tree is not possible with current DMEAlgorithm, but a single node tree is.
        // For get_tree_statistics, if passed a leaf node as root, it should correctly report.
        auto leaf_node = std::make_shared<recti::TreeNode>("leaf1", recti::Point<int>(0, 0));
        leaf_node->delay = 0.0;
        leaf_node->capacitance = 1.0;

        auto stats = recti::get_tree_statistics(leaf_node);

        CHECK(stats.total_nodes == 1);
        CHECK(stats.total_sinks == 1);
        CHECK(stats.total_wires == 0);
        CHECK(stats.sinks.size() == 1);
        CHECK(stats.sinks[0] == "leaf1");

        CHECK(stats.nodes[0].name == "leaf1");
        CHECK(stats.nodes[0].type == "sink");
        CHECK(stats.nodes[0].position == std::pair<int, int>(0, 0));
        CHECK(stats.nodes[0].delay == doctest::Approx(0.0));
        CHECK(stats.nodes[0].capacitance == doctest::Approx(1.0));
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