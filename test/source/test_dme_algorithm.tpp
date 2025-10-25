#include <doctest/doctest.h>
#include <fmt/core.h>

#include <ldsgen/ilds.hpp>  // for VdCorput
#include <recti/dme_algorithm.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <string>
#include <variant>
#include <vector>
#include <sstream>

using namespace recti;

TEST_SUITE("DME Algorithm Tests") {

    TEST_CASE("Sink class") {
        SUBCASE("Basic constructor and accessors") {
            Sink sink("s1", Point<int, int>(10, 20), 1.5f);
            CHECK(sink.name() == "s1");
            CHECK(sink.position() == Point<int, int>(10, 20));
            CHECK(sink.capacitance() == 1.5f);
        }

        SUBCASE("String representation") {
            Sink sink("s1", Point<int, int>(10, 20), 1.5f);
            std::ostringstream oss;
            oss << sink;
            CHECK(oss.str() == "Sink(name=s1, position=(10, 20), capacitance=1.5)");
        }
    }

    TEST_CASE("TreeNode class") {
        SUBCASE("Basic constructor and accessors") {
            TreeNode node("n1", Point<int, int>(30, 40));
            CHECK(node.name() == "n1");
            CHECK(node.position() == Point<int, int>(30, 40));
            CHECK_FALSE(node.left().has_value());
            CHECK_FALSE(node.right().has_value());
            CHECK_FALSE(node.parent().has_value());
            CHECK(node.wire_length() == 0);
            CHECK(node.delay() == 0.0f);
            CHECK(node.capacitance() == 0.0f);
            CHECK_FALSE(node.need_elongation());
        }

        SUBCASE("String representation") {
            TreeNode node("n1", Point<int, int>(30, 40));
            std::ostringstream oss;
            oss << node;
            CHECK(oss.str() == "TreeNode(name=n1, position=(30, 40))");
        }
    }

    TEST_CASE("LinearDelayCalculator class") {
        LinearDelayCalculator calc(0.5f, 0.2f);

        SUBCASE("Constructor and accessors") {
            CHECK(calc.delay_per_unit() == 0.5f);
        }

        SUBCASE("calculate_wire_delay") {
            CHECK(calc.calculate_wire_delay(10, 5.0f) == 5.0f);
        }

        SUBCASE("calculate_wire_delay_per_unit") {
            CHECK(calc.calculate_wire_delay_per_unit(5.0f) == 0.5f);
        }

        SUBCASE("calculate_wire_capacitance") {
            CHECK(calc.calculate_wire_capacitance(10) == 2.0f);
        }

        SUBCASE("calculate_tapping_point") {
            TreeNode left("left", Point<int, int>(0, 0), 1.0f);
            TreeNode right("right", Point<int, int>(10, 10), 1.0f);
            right.set_delay(2.0f);
            auto [extend_left, delay_left] = calc.calculate_tapping_point(&left, &right, 10);
            CHECK(extend_left == 6); // (2.0 / 0.5 + 10) / 2 = 7, rounded to 6
            CHECK(delay_left == 3.0f); // 0.0 + 6 * 0.5
            CHECK(left.wire_length() == 6);
            CHECK(right.wire_length() == 4);
        }
    }

    TEST_CASE("ElmoreDelayCalculator class") {
        ElmoreDelayCalculator calc(0.1f, 0.2f);

        SUBCASE("Constructor and accessors") {
            CHECK(calc.unit_resistance() == 0.1f);
        }

        SUBCASE("calculate_wire_delay") {
            CHECK(calc.calculate_wire_delay(10, 5.0f) == 6.0f); // 0.1 * 10 * (0.2 * 10 / 2 + 5.0)
        }

        SUBCASE("calculate_wire_delay_per_unit") {
            CHECK(calc.calculate_wire_delay_per_unit(5.0f) == 0.51f); // 0.1 * (0.2 / 2 + 5.0)
        }

        SUBCASE("calculate_wire_capacitance") {
            CHECK(calc.calculate_wire_capacitance(10) == 2.0f); // 0.2 * 10
        }

        SUBCASE("calculate_tapping_point") {
            TreeNode left("left", Point<int, int>(0, 0), 1.0f);
            TreeNode right("right", Point<int, int>(10, 10), 1.0f);
            right.set_delay(2.0f);
            auto [extend_left, delay_left] = calc.calculate_tapping_point(&left, &right, 10);
            // z = (2.0 + 1.0 * (1.0 + 0.2 * 10 / 2)) / (1.0 * (0.2 * 10 + 1.0 + 1.0)) = 0.75
            // extend_left = round(0.75 * 10) = 8
            CHECK(extend_left == 8);
            // delay_left = 0.0 + 0.8 * (0.8 * 0.2 / 2 + 1.0) = 0.88
            CHECK(std::abs(delay_left - 0.88f) < 1e-5f);
            CHECK(left.wire_length() == 8);
            CHECK(right.wire_length() == 2);
        }
    }

    // TEST_CASE("DMEAlgorithm class") {
    //     SUBCASE("Constructor") {
    //         std::vector<Sink> sinks = {Sink("s1", Point<int, int>(10, 20), 1.0f)};
    //         auto calc = std::make_unique<LinearDelayCalculator>(0.5f);
    //         DMEAlgorithm dme(sinks, std::move(calc));
    //         // CHECK(typeid(*dme.delay_calculator_) == typeid(LinearDelayCalculator));

    //         auto calc2 = std::make_unique<ElmoreDelayCalculator>(0.1f, 0.2f);
    //         DMEAlgorithm dme2(sinks, std::move(calc2));
    //         // CHECK(typeid(*dme2.delay_calculator_) == typeid(ElmoreDelayCalculator));
    //     }

    //     SUBCASE("Empty sinks") {
    //         std::vector<Sink> sinks;
    //         auto calc = std::make_unique<LinearDelayCalculator>();
    //         CHECK_THROWS_AS(DMEAlgorithm(sinks, std::move(calc)), std::invalid_argument);
    //     }
    // }

    // TEST_CASE("get_tree_statistics function") {
    //     TreeNode s1("s1", Point<int, int>(10, 20));
    //     TreeNode s2("s2", Point<int, int>(30, 40));
    //     TreeNode root("n1", Point<int, int>(20, 30));
    //     root.set_left(&s1);
    //     root.set_right(&s2);
    //     s1.set_parent(&root);
    //     s2.set_parent(&root);

    //     auto stats = get_tree_statistics(&root);
    //     CHECK(std::get<int>(stats.at("total_nodes")) == 3);
    //     CHECK(std::get<int>(stats.at("total_sinks")) == 2);
    //     CHECK(std::get<int>(stats.at("total_wires")) == 2);
    //     auto nodes = std::get<std::vector<std::unordered_map<std::string, std::variant<std::string, std::tuple<int, int>, float>>>>(stats.at("nodes"));
    //     CHECK(nodes.size() == 3);
    //     CHECK(std::get<std::string>(nodes[0].at("name")) == "n1");
    //     CHECK(std::get<std::string>(nodes[1].at("name")) == "s1");
    //     CHECK(std::get<std::string>(nodes[2].at("name")) == "s2");
    //     CHECK(std::get<std::string>(nodes[1].at("type")) == "sink");
    //     CHECK(std::get<std::string>(nodes[0].at("type")) == "internal");
    // }

}
