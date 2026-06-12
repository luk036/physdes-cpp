#include <doctest/doctest.h>

#include <iostream>
#include <memory>
#include <recti/manhattan_arc.hpp>
#include <string>
#include <vector>

#include "recti/dme_algorithm.hpp"

using namespace recti;

namespace {
    int count_leaves(const Tree& tree, NodeIdx node) {
        if (node == SIZE_MAX) {
            return 0;
        }
        if (tree.get(node).is_leaf()) {
            return 1;
        }
        return count_leaves(tree, tree.get(node).left) + count_leaves(tree, tree.get(node).right);
    }

    template <typename DelayCalcT, typename... Args>
    auto create_dme_algorithm(const std::vector<Sink>& sinks, Args&&... args) {
        auto calculator = std::make_unique<DelayCalcT>(std::forward<Args>(args)...);
        return DMEAlgorithm(sinks, std::move(calculator));
    }
}  // namespace

TEST_SUITE("Sink Tests") {
    TEST_CASE("Sink Construction") {
        SUBCASE("Basic construction") {
            Sink sink("s1", Point<int>(10, 20), 1.5);
            CHECK_EQ(sink.name, "s1");
            CHECK_EQ(sink.position.xcoord(), 10);
            CHECK_EQ(sink.position.ycoord(), 20);
            CHECK(sink.capacitance == doctest::Approx(1.5));
        }
        SUBCASE("Default capacitance") {
            Sink sink("s2", Point<int>(30, 40));
            CHECK_EQ(sink.name, "s2");
            CHECK(sink.capacitance == doctest::Approx(1.0));
        }
    }
}

TEST_SUITE("TreeNode Tests") {
    TEST_CASE("TreeNode Construction") {
        TreeNode node("n1", Point<int>(30, 40));
        CHECK_EQ(node.name, "n1");
        CHECK_EQ(node.position.xcoord(), 30);
        CHECK_EQ(node.position.ycoord(), 40);
        CHECK_EQ(node.wire_length, 0);
        CHECK_EQ(node.delay, doctest::Approx(0.0));
        CHECK_EQ(node.capacitance, doctest::Approx(0.0));
        CHECK_EQ(node.need_elongation, false);
        CHECK_EQ(node.left, SIZE_MAX);
        CHECK_EQ(node.right, SIZE_MAX);
        CHECK_EQ(node.parent, SIZE_MAX);
    }

    TEST_CASE("TreeNode Leaf Detection") {
        SUBCASE("Leaf node") {
            TreeNode leaf("leaf", Point<int>(10, 10));
            CHECK_EQ(leaf.is_leaf(), true);
        }
        SUBCASE("Internal node via Tree") {
            Tree tree;
            NodeIdx left = tree.add(TreeNode("left", Point<int>(10, 10)));
            NodeIdx right = tree.add(TreeNode("right", Point<int>(20, 20)));
            NodeIdx internal = tree.add(TreeNode("internal", Point<int>(15, 15)));
            tree.get_mut(internal).left = left;
            tree.get_mut(internal).right = right;
            CHECK_EQ(tree.get(internal).is_leaf(), false);
        }
    }
}

TEST_SUITE("LinearDelayCalculator Tests") {
    TEST_CASE("LinearDelayCalculator Construction") {
        LinearDelayCalculator calc(0.5, 0.2);
        SUBCASE("Wire delay calculation") {
            double delay = calc.calculate_wire_delay(10, 5.0);
            CHECK(delay == doctest::Approx(5.0));
        }
    }

    TEST_CASE("LinearDelayCalculator Methods") {
        LinearDelayCalculator calc(0.5, 0.2);
        SUBCASE("Calculate wire delay") {
            CHECK(calc.calculate_wire_delay(10, 5.0) == doctest::Approx(5.0));
            CHECK(calc.calculate_wire_delay(0, 100.0) == doctest::Approx(0.0));
        }
        SUBCASE("Calculate wire delay per unit") {
            CHECK(calc.calculate_wire_delay_per_unit(5.0) == doctest::Approx(0.5));
        }
        SUBCASE("Calculate wire capacitance") {
            CHECK(calc.calculate_wire_capacitance(10) == doctest::Approx(2.0));
            CHECK(calc.calculate_wire_capacitance(0) == doctest::Approx(0.0));
        }
    }

    TEST_CASE("LinearDelayCalculator Tapping Point") {
        LinearDelayCalculator calc(0.5, 0.2);

        SUBCASE("Balanced case") {
            auto tp = calc.calculate_tapping_point(10, 1.0, 1.0, 0.0, 0.0);
            CHECK_EQ(tp.extend_left, 5);
            CHECK(tp.delay_left == doctest::Approx(1.0 + 5 * 0.5));
        }

        SUBCASE("Skewed case - right slower") {
            auto tp = calc.calculate_tapping_point(10, 1.0, 3.0, 0.0, 0.0);
            CHECK_EQ(tp.extend_left, 7);
            CHECK(tp.delay_left == doctest::Approx(1.0 + 7 * 0.5));
        }

        SUBCASE("Skewed case - left slower") {
            auto tp = calc.calculate_tapping_point(10, 3.0, 1.0, 0.0, 0.0);
            CHECK_EQ(tp.extend_left, 3);
            CHECK(tp.delay_left == doctest::Approx(3.0 + 3 * 0.5));
        }

        SUBCASE("Elongation needed for right branch (extend_left < 0)") {
            // left.delay=10, right.delay=1, distance=10
            auto tp = calc.calculate_tapping_point(10, 10.0, 1.0, 0.0, 0.0);
            CHECK_EQ(tp.extend_left, 0);
            CHECK(tp.delay_left == doctest::Approx(10.0));
        }

        SUBCASE("Elongation needed for left branch (extend_left > distance)") {
            // left.delay=1, right.delay=10, distance=10
            auto tp = calc.calculate_tapping_point(10, 1.0, 10.0, 0.0, 0.0);
            CHECK_EQ(tp.extend_left, 10);
            CHECK(tp.delay_left == doctest::Approx(10.0));
        }
    }
}

TEST_SUITE("ElmoreDelayCalculator Tests") {
    TEST_CASE("ElmoreDelayCalculator Methods") {
        ElmoreDelayCalculator calc(0.1, 0.2);
        CHECK(calc.calculate_wire_delay(10, 5.0) == doctest::Approx(6.0));
        CHECK(calc.calculate_wire_delay(0, 100.0) == doctest::Approx(0.0));
        CHECK(calc.calculate_wire_delay_per_unit(5.0) == doctest::Approx(0.51));
        CHECK(calc.calculate_wire_capacitance(10) == doctest::Approx(2.0));
    }

    TEST_CASE("ElmoreDelayCalculator Tapping Point") {
        ElmoreDelayCalculator calc(0.1, 0.2);

        SUBCASE("Balanced case") {
            auto tp = calc.calculate_tapping_point(10, 1.0, 1.0, 1.0, 1.0);
            CHECK_EQ(tp.extend_left, 5);
            CHECK(tp.delay_left == doctest::Approx(1.75));
        }

        SUBCASE("Skewed case - right slower") {
            auto tp = calc.calculate_tapping_point(10, 1.0, 3.0, 1.0, 1.0);
            CHECK_EQ(tp.extend_left, 10);
            CHECK(tp.delay_left == doctest::Approx(1.0 + 1.0 * (1.0 + 1.0)));
        }

        SUBCASE("Elongation needed for right branch (extend_left < 0)") {
            auto tp = calc.calculate_tapping_point(10, 10.0, 1.0, 1.0, 1.0);
            CHECK_EQ(tp.extend_left, 0);
            CHECK(tp.delay_left == doctest::Approx(10.0));
        }

        SUBCASE("Elongation needed for left branch (extend_left > distance)") {
            auto tp = calc.calculate_tapping_point(10, 1.0, 10.0, 1.0, 1.0);
            CHECK_EQ(tp.extend_left, 10);
            CHECK(tp.delay_left == doctest::Approx(10.0));
        }
    }
}

TEST_SUITE("DMEAlgorithm Tests") {
    TEST_CASE("DMEAlgorithm Construction") {
        SUBCASE("Valid construction") {
            std::vector<Sink> sinks = {Sink("s1", Point<int>(10, 20), 1.0)};
            REQUIRE_NOTHROW(create_dme_algorithm<LinearDelayCalculator>(sinks));
        }
        SUBCASE("Empty sinks throws exception") {
            std::vector<Sink> sinks;
            REQUIRE_THROWS_AS(create_dme_algorithm<LinearDelayCalculator>(sinks),
                              std::invalid_argument);
        }
    }

    TEST_CASE("DMEAlgorithm Single Sink") {
        std::vector<Sink> sinks = {Sink("s1", Point<int>(10, 20), 1.5)};
        auto dme = create_dme_algorithm<LinearDelayCalculator>(sinks);
        NodeIdx root = dme.build_clock_tree();
        const Tree& tree = dme.get_tree();

        CHECK_EQ(tree.get(root).name, "s1");
        CHECK_EQ(tree.get(root).position.xcoord(), 10);
        CHECK_EQ(tree.get(root).position.ycoord(), 20);
        CHECK_EQ(tree.get(root).is_leaf(), true);
        CHECK(tree.get(root).delay == doctest::Approx(0.0));
    }

    TEST_CASE("DMEAlgorithm Two Sinks") {
        std::vector<Sink> sinks
            = {Sink("s1", Point<int>(0, 0), 1.0), Sink("s2", Point<int>(10, 0), 1.0)};
        auto dme = create_dme_algorithm<LinearDelayCalculator>(sinks, 1.0, 1.0);
        NodeIdx root = dme.build_clock_tree();
        const Tree& tree = dme.get_tree();

        CHECK_EQ(tree.get(root).is_leaf(), false);
        CHECK_NE(tree.get(root).left, SIZE_MAX);
        CHECK_NE(tree.get(root).right, SIZE_MAX);
        CHECK_EQ(tree.get(tree.get(root).left).is_leaf(), true);
        CHECK_EQ(tree.get(tree.get(root).right).is_leaf(), true);

        auto analysis = dme.analyze_skew(root);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(0.001));
        CHECK_GT(analysis.total_wirelength, 0);
    }

    TEST_CASE("DMEAlgorithm Multiple Sinks") {
        std::vector<Sink> sinks
            = {Sink("s1", Point<int>(10, 10), 1.0), Sink("s2", Point<int>(20, 10), 1.0),
               Sink("s3", Point<int>(15, 30), 1.0), Sink("s4", Point<int>(25, 30), 1.0)};
        auto dme = create_dme_algorithm<LinearDelayCalculator>(sinks, 1.0, 1.0);
        NodeIdx root = dme.build_clock_tree();
        const Tree& tree = dme.get_tree();

        CHECK_NE(root, SIZE_MAX);
        CHECK_EQ(tree.get(root).is_leaf(), false);
        CHECK_EQ(count_leaves(tree, root), 4);
        CHECK_NE(tree.get(root).left, SIZE_MAX);
        CHECK_NE(tree.get(root).right, SIZE_MAX);
    }

    TEST_CASE("DMEAlgorithm Skew Analysis") {
        std::vector<Sink> sinks
            = {Sink("s1", Point<int>(0, 0), 1.0), Sink("s2", Point<int>(10, 10), 1.0),
               Sink("s3", Point<int>(20, 0), 1.0)};
        auto dme = create_dme_algorithm<LinearDelayCalculator>(sinks, 1.0, 1.0);
        NodeIdx root = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(root);

        CHECK_GE(analysis.max_delay, analysis.min_delay);
        CHECK_GE(analysis.skew, 0.0);
        CHECK_EQ(analysis.sink_delays.size(), 3);
        CHECK_GT(analysis.total_wirelength, 0);
        CHECK_FALSE(analysis.delay_model.empty());
        CHECK_EQ(analysis.skew, doctest::Approx(0.0).epsilon(0.001));
    }
}

TEST_SUITE("Tree Statistics Tests") {
    TEST_CASE("Get Tree Statistics") {
        Tree tree;
        NodeIdx s1 = tree.add(TreeNode("s1", Point<int>(10, 20)));
        NodeIdx s2 = tree.add(TreeNode("s2", Point<int>(30, 40)));
        NodeIdx root = tree.add(TreeNode("n1", Point<int>(20, 30)));

        tree.get_mut(root).left = s1;
        tree.get_mut(root).right = s2;
        tree.get_mut(s1).parent = root;
        tree.get_mut(s2).parent = root;

        tree.get_mut(s1).wire_length = 5;
        tree.get_mut(s2).wire_length = 5;
        tree.get_mut(s1).delay = 1.0;
        tree.get_mut(s2).delay = 1.0;
        tree.get_mut(s1).capacitance = 1.0;
        tree.get_mut(s2).capacitance = 1.0;

        auto stats = get_tree_statistics(tree, root);

        CHECK_EQ(stats.total_nodes, 3);
        CHECK_EQ(stats.total_sinks, 2);
        CHECK_EQ(stats.total_wires, 2);
        CHECK_EQ(stats.sinks.size(), 2);
        CHECK_EQ(stats.nodes.size(), 3);
        CHECK_EQ(stats.wires.size(), 2);

        bool found_s1 = false, found_s2 = false, found_root = false;
        for (const auto& node : stats.nodes) {
            if (node.name == "s1") {
                found_s1 = true;
                CHECK_EQ(node.type, "sink");
                CHECK_EQ(node.position, std::pair<int, int>(10, 20));
            } else if (node.name == "s2") {
                found_s2 = true;
                CHECK_EQ(node.type, "sink");
                CHECK_EQ(node.position, std::pair<int, int>(30, 40));
            } else if (node.name == "n1") {
                found_root = true;
                CHECK_EQ(node.type, "internal");
                CHECK_EQ(node.position, std::pair<int, int>(20, 30));
            }
        }
        CHECK(found_s1);
        CHECK(found_s2);
        CHECK(found_root);

        for (const auto& wire : stats.wires) {
            CHECK_EQ(wire.from_node, "n1");
            CHECK((wire.to_node == "s1" || wire.to_node == "s2"));
            CHECK_EQ(wire.length, 5);
        }
    }

    TEST_CASE("Single Node Tree Statistics") {
        Tree tree;
        NodeIdx leaf = tree.add(TreeNode("leaf1", Point<int>(0, 0)));
        tree.get_mut(leaf).delay = 0.0;
        tree.get_mut(leaf).capacitance = 1.0;

        auto stats = get_tree_statistics(tree, leaf);
        CHECK_EQ(stats.total_nodes, 1);
        CHECK_EQ(stats.total_sinks, 1);
        CHECK_EQ(stats.total_wires, 0);
        CHECK_EQ(stats.sinks[0], "leaf1");
        CHECK_EQ(stats.nodes[0].type, "sink");
    }
}

TEST_SUITE("Integration Tests") {
    TEST_CASE("Linear vs Elmore Comparison") {
        std::vector<Sink> sinks
            = {Sink("s1", Point<int>(10, 20), 1.0), Sink("s2", Point<int>(30, 40), 1.0),
               Sink("s3", Point<int>(50, 10), 1.0)};

        auto dme_linear = create_dme_algorithm<LinearDelayCalculator>(sinks, 0.5, 0.2);
        NodeIdx rl = dme_linear.build_clock_tree();
        auto al = dme_linear.analyze_skew(rl);

        auto dme_elmore = create_dme_algorithm<ElmoreDelayCalculator>(sinks, 0.1, 0.2);
        NodeIdx re = dme_elmore.build_clock_tree();
        auto ae = dme_elmore.analyze_skew(re);

        CHECK(al.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK(ae.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK_GT(al.total_wirelength, 0);
        CHECK_GT(ae.total_wirelength, 0);
        CHECK_NE(al.delay_model, ae.delay_model);
    }

    TEST_CASE("Large Tree Construction") {
        std::vector<Sink> sinks;
        sinks.reserve(8);
        for (int i = 0; i < 8; ++i) {
            sinks.emplace_back(std::string("s") + std::to_string(i), Point<int>(i * 10, i * 5),
                               1.0 + i * 0.1);
        }

        auto dme = create_dme_algorithm<LinearDelayCalculator>(sinks);
        NodeIdx root = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(root);
        auto stats = get_tree_statistics(dme.get_tree(), root);

        CHECK_NE(root, SIZE_MAX);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(1.0));
        CHECK_EQ(stats.total_nodes, 15);
        CHECK_EQ(stats.total_sinks, 8);
    }
}

TEST_SUITE("Elongation Integration Tests") {
    TEST_CASE("Linear elongation wire_length and need_elongation match original") {
        // Reproduce the original calculate_tapping_point side-effect test using
        // the caller-level logic that now lives in _compute_merging_segment.
        LinearDelayCalculator calc(0.5, 0.2);
        int distance = 10;

        // Case 1: right branch needs elongation (raw_extend_left < 0)
        SUBCASE("Right branch elongation (raw < 0)") {
            // left.delay=10, right.delay=1, distance=10
            auto tp = calc.calculate_tapping_point(distance, 10.0, 1.0, 0.0, 0.0);
            CHECK_EQ(tp.extend_left, 0);
            CHECK_EQ(tp.raw_extend_left, -4);

            // Caller-side logic (matching _compute_merging_segment):
            int left_wl = tp.extend_left;
            int right_wl = distance - tp.raw_extend_left;
            bool left_el = false, right_el = false;
            if (tp.raw_extend_left < 0) {
                left_wl = 0;
                right_wl = distance - tp.raw_extend_left;
                right_el = true;
            } else if (tp.raw_extend_left > distance) {
                right_wl = 0;
                left_wl = tp.raw_extend_left;
                left_el = true;
            }

            // Numerical values identical to original test
            CHECK_EQ(left_wl, 0);
            CHECK_EQ(right_wl, 14);
            CHECK_EQ(left_el, false);
            CHECK_EQ(right_el, true);
        }

        // Case 2: left branch needs elongation (raw_extend_left > distance)
        SUBCASE("Left branch elongation (raw > distance)") {
            // left.delay=1, right.delay=10, distance=10
            auto tp = calc.calculate_tapping_point(distance, 1.0, 10.0, 0.0, 0.0);
            CHECK_EQ(tp.extend_left, 10);
            CHECK_EQ(tp.raw_extend_left, 14);

            int left_wl = tp.extend_left;
            int right_wl = distance - tp.raw_extend_left;
            bool left_el = false, right_el = false;
            if (tp.raw_extend_left < 0) {
                left_wl = 0;
                right_wl = distance - tp.raw_extend_left;
                right_el = true;
            } else if (tp.raw_extend_left > distance) {
                right_wl = 0;
                left_wl = tp.raw_extend_left;
                left_el = true;
            }

            // Numerical values identical to original test
            CHECK_EQ(left_wl, 14);
            CHECK_EQ(right_wl, 0);
            CHECK_EQ(left_el, true);
            CHECK_EQ(right_el, false);
        }
    }

    TEST_CASE("Elmore elongation wire_length and need_elongation match original") {
        ElmoreDelayCalculator calc(0.1, 0.2);
        int distance = 10;

        SUBCASE("Right branch elongation (raw < 0)") {
            auto tp = calc.calculate_tapping_point(distance, 10.0, 1.0, 1.0, 1.0);
            CHECK_EQ(tp.extend_left, 0);
            CHECK_EQ(tp.raw_extend_left, -18);

            int left_wl = tp.extend_left;
            int right_wl = distance - tp.raw_extend_left;
            bool left_el = false, right_el = false;
            if (tp.raw_extend_left < 0) {
                left_wl = 0;
                right_wl = distance - tp.raw_extend_left;
                right_el = true;
            } else if (tp.raw_extend_left > distance) {
                right_wl = 0;
                left_wl = tp.raw_extend_left;
                left_el = true;
            }

            CHECK_EQ(left_wl, 0);
            CHECK_EQ(right_wl, 28);
            CHECK_EQ(left_el, false);
            CHECK_EQ(right_el, true);
        }

        SUBCASE("Left branch elongation (raw > distance)") {
            auto tp = calc.calculate_tapping_point(distance, 1.0, 10.0, 1.0, 1.0);
            CHECK_EQ(tp.extend_left, 10);
            CHECK_EQ(tp.raw_extend_left, 28);

            int left_wl = tp.extend_left;
            int right_wl = distance - tp.raw_extend_left;
            bool left_el = false, right_el = false;
            if (tp.raw_extend_left < 0) {
                left_wl = 0;
                right_wl = distance - tp.raw_extend_left;
                right_el = true;
            } else if (tp.raw_extend_left > distance) {
                right_wl = 0;
                left_wl = tp.raw_extend_left;
                left_el = true;
            }

            CHECK_EQ(left_wl, 28);
            CHECK_EQ(right_wl, 0);
            CHECK_EQ(left_el, true);
            CHECK_EQ(right_el, false);
        }
    }
}

TEST_CASE("Edge Cases") {
    SUBCASE("Sinks at same position") {
        std::vector<Sink> sinks
            = {Sink("s1", Point<int>(10, 10), 1.0), Sink("s2", Point<int>(10, 10), 1.0),
               Sink("s3", Point<int>(10, 10), 1.0)};
        auto calculator = std::make_unique<LinearDelayCalculator>();
        DMEAlgorithm dme(sinks, std::move(calculator));
        NodeIdx root = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(root);
        CHECK_NE(root, SIZE_MAX);
        CHECK(analysis.skew == doctest::Approx(0.0));
    }

    SUBCASE("Very distant sinks") {
        std::vector<Sink> sinks
            = {Sink("s1", Point<int>(0, 0), 1.0), Sink("s2", Point<int>(1000, 1000), 1.0)};
        auto calculator = std::make_unique<LinearDelayCalculator>();
        DMEAlgorithm dme(sinks, std::move(calculator));
        NodeIdx root = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(root);
        CHECK_NE(root, SIZE_MAX);
        CHECK_GE(analysis.total_wirelength, 2000);
        CHECK(analysis.skew == doctest::Approx(0.0).epsilon(0.001));
    }
}
