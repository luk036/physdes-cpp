#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST...

#include <iostream>              // for operator<<, ostream, basic_ostream
#include <recti/halton_int.hpp>  // for recti
#include <recti/visualize_clock_tree.hpp>   // for ManhattanArc, operator+, operator-

#include "recti/interval.hpp"  // for min_dist, overlap, interval

using namespace recti;


TEST_SUITE("Clock Tree Visualization Tests") {

    TEST_CASE("ClockTreeVisualizer class") {
        SUBCASE("Constructor and accessors") {
            ClockTreeVisualizer viz(10, 5);
            CHECK(viz.margin() == 10);
            CHECK(viz.node_radius() == 5);
        }

        SUBCASE("visualize_tree") {
            TreeNode root("root", Point<int, int>(50, 50));
            std::vector<Sink> sinks = {Sink("s1", Point<int, int>(10, 20)), Sink("s2", Point<int, int>(30, 40))};
            root.set_left(new TreeNode("s1", Point<int, int>(10, 20), 1.0f));
            root.set_right(new TreeNode("s2", Point<int, int>(30, 40), 1.0f));
            root.left().value()->set_parent(&root);
            root.right().value()->set_parent(&root);
            root.left().value()->set_wire_length(20);
            root.right().value()->set_wire_length(30);
            root.left().value()->set_delay(1.0f);
            root.right().value()->set_delay(1.5f);
            root.left().value()->set_capacitance(1.0f);
            root.right().value()->set_capacitance(1.0f);

            ClockTreeVisualizer viz;
            auto svg = viz.visualize_tree(&root, sinks, "");
            CHECK(svg.find("<svg width=\"800\" height=\"600\"") != std::string::npos);
            CHECK(svg.find("class=\"node-label\"") != std::string::npos);
            CHECK(svg.find("class=\"delay-label\"") != std::string::npos);
            CHECK(svg.find("class=\"wire-label\"") != std::string::npos);
            std::ofstream file("visualize_tree.svg");
            file << svg;
            file.close();
        }

        SUBCASE("visualize_tree with analysis") {
            TreeNode root("root", Point<int, int>(50, 50));
            std::vector<Sink> sinks = {Sink("s1", Point<int, int>(10, 20))};
            std::unordered_map<std::string, std::variant<float, std::string, std::vector<float>>> analysis = {
                {"max_delay", 5.0f},
                {"min_delay", 4.0f},
                {"skew", 1.0f},
                {"total_wirelength", 100.0f},
                {"sink_delays", std::vector<float>{5.0f}},
                {"delay_model", std::string("LinearDelayCalculator")}
            };
            ClockTreeVisualizer viz;
            auto svg = viz.visualize_tree(&root, sinks, "", 800, 600, analysis);
            CHECK(svg.find("Clock Tree Analysis") != std::string::npos);
            CHECK(svg.find("Delay Model: LinearDelayCalculator") != std::string::npos);
            CHECK(svg.find("Max Delay: 5") != std::string::npos);
            std::ofstream file("visualize_tree_with_analysis.svg");
            file << svg;
            file.close();

        }
    }

    // TEST_CASE("create_comparison_visualization") {
    //     TreeNode tree1("root1", Point<int, int>(50, 50));
    //     TreeNode tree2("root2", Point<int, int>(150, 50));
    //     std::vector<Sink> sinks = {Sink("s1", Point<int, int>(10, 20)), Sink("s2", Point<int, int>(30, 40))};
    //     std::unordered_map<std::string, std::variant<float, std::string, std::vector<float>>> analysis = {
    //         {"skew", 0.1f},
    //         {"max_delay", 5.0f},
    //         {"total_wirelength", 100.0f}
    //     };
    //     std::vector<std::unordered_map<std::string, std::variant<const TreeNode*, std::vector<Sink>, std::unordered_map<std::string, std::variant<float, std::string, std::vector<float>>>, std::string>>> data = {
    //         {
    //             {"tree", &tree1},
    //             {"sinks", sinks},
    //             {"analysis", analysis},
    //             {"title", std::string("Linear Model")}
    //         },
    //         {
    //             {"tree", &tree2},
    //             {"sinks", sinks},
    //             {"analysis", analysis},
    //             {"title", std::string("Elmore Model")}
    //         }
    //     };

    //     auto svg = create_comparison_visualization(data, "");
    //     CHECK(svg.find("Linear Model") != std::string::npos);
    //     CHECK(svg.find("Elmore Model") != std::string::npos);
    //     CHECK(svg.find("<svg width=\"1200\" height=\"800\"") != std::string::npos);
    // }

    // TEST_CASE("create_delay_model_comparison") {
    //     TreeNode tree1("root1", Point<int, int>(50, 50));
    //     TreeNode tree2("root2", Point<int, int>(150, 50));
    //     std::vector<Sink> sinks = {Sink("s1", Point<int, int>(10, 20))};
    //     std::unordered_map<std::string, std::variant<float, std::string, std::vector<float>>> analysis = {
    //         {"skew", 0.1f},
    //         {"max_delay", 5.0f},
    //         {"total_wirelength", 100.0f}
    //     };
    //     std::unordered_map<std::string, std::variant<const TreeNode*, std::vector<Sink>, std::unordered_map<std::string, std::variant<float, std::string, std::vector<float>>>, std::string>> linear_data = {
    //         {"tree", &tree1},
    //         {"sinks", sinks},
    //         {"analysis", analysis},
    //         {"title", std::string("Linear Model")}
    //     };
    //     std::unordered_map<std::string, std::variant<const TreeNode*, std::vector<Sink>, std::unordered_map<std::string, std::variant<float, std::string, std::vector<float>>>, std::string>> elmore_data = {
    //         {"tree", &tree2},
    //         {"sinks", sinks},
    //         {"analysis", analysis},
    //         {"title", std::string("Elmore Model")}
    //     };

    //     auto svg = create_delay_model_comparison(linear_data, elmore_data, "");
    //     CHECK(svg.find("Linear Delay Model") != std::string::npos);
    //     CHECK(svg.find("Elmore Delay Model") != std::string::npos);
    // }

}
