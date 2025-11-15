#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase, TEST...

#include <recti/halton_int.hpp>            // for recti
#include <recti/visualize_clock_tree.hpp>  // for ManhattanArc, operator+, operator-


#include <doctest/doctest.h>
#include <memory>
#include <vector>
#include <string>
#include <filesystem>


TEST_SUITE("ClockTreeVisualizer Tests") {
    // TEST_CASE("ClockTreeVisualizer Construction") {
    //     SUBCASE("Default parameters") {
    //         recti::ClockTreeVisualizer viz;
            
    //         CHECK(viz.margin == 50);
    //         CHECK(viz.node_radius == 8);
    //         CHECK(viz.wire_width == 2);
    //         CHECK(viz.sink_color == "#4CAF50");
    //         CHECK(viz.internal_color == "#2196F3");
    //         CHECK(viz.root_color == "#F44336");
    //     }
        
    //     SUBCASE("Custom parameters") {
    //         recti::ClockTreeVisualizer viz(
    //             10, 5, 1, 
    //             "#FF0000", "#00FF00", "#0000FF",
    //             "#CCCCCC", "#333333"
    //         );
            
    //         CHECK(viz.margin == 10);
    //         CHECK(viz.node_radius == 5);
    //         CHECK(viz.wire_width == 1);
    //         CHECK(viz.sink_color == "#FF0000");
    //         CHECK(viz.internal_color == "#00FF00");
    //         CHECK(viz.root_color == "#0000FF");
    //     }
    // }
    
    TEST_CASE("Single Sink Visualization") {
        // Create a simple single-sink tree
        auto sink = std::make_shared<recti::TreeNode>("s1", recti::Point<int>(100, 100));
        sink->capacitance = 1.5;
        sink->delay = 0.0;
        
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(100, 100), 1.5)
        };
        
        recti::ClockTreeVisualizer visualizer;
        std::string svg_content = visualizer.visualize_tree(
            sink, sinks, "test_single_sink.svg", 400, 300
        );
        
        // Check that SVG was generated
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        CHECK(svg_content.find("</svg>") != std::string::npos);
        
        // Check that the sink node is present
        CHECK(svg_content.find("s1") != std::string::npos);
        CHECK(svg_content.find("circle") != std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_single_sink.svg"));
        
        // Cleanup
        std::filesystem::remove("test_single_sink.svg");
    }
    
    TEST_CASE("Two Sink Tree Visualization") {
        // Create a simple two-sink tree
        auto sink1 = std::make_shared<recti::TreeNode>("s1", recti::Point<int>(50, 50));
        auto sink2 = std::make_shared<recti::TreeNode>("s2", recti::Point<int>(150, 150));
        auto root = std::make_shared<recti::TreeNode>("root", recti::Point<int>(100, 100));
        
        sink1->parent = root;
        sink2->parent = root;
        root->left = sink1;
        root->right = sink2;
        
        sink1->wire_length = 10;
        sink2->wire_length = 10;
        sink1->delay = 5.0;
        sink2->delay = 5.0;
        sink1->capacitance = 1.0;
        sink2->capacitance = 1.0;
        root->capacitance = 2.0;
        
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(50, 50), 1.0),
            recti::Sink("s2", recti::Point<int>(150, 150), 1.0)
        };
        
        recti::ClockTreeVisualizer visualizer;
        std::string svg_content = visualizer.visualize_tree(
            root, sinks, "test_two_sinks.svg", 600, 400
        );
        
        // Basic SVG structure checks
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        CHECK(svg_content.find("</svg>") != std::string::npos);
        
        // Check for all nodes
        CHECK(svg_content.find("s1") != std::string::npos);
        CHECK(svg_content.find("s2") != std::string::npos);
        CHECK(svg_content.find("root") != std::string::npos);
        
        // Check for wire elements
        CHECK(svg_content.find("line") != std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_two_sinks.svg"));
        
        // Cleanup
        std::filesystem::remove("test_two_sinks.svg");
    }
    
    TEST_CASE("Visualization with Analysis") {
        auto sink1 = std::make_shared<recti::TreeNode>("s1", recti::Point<int>(50, 50));
        auto sink2 = std::make_shared<recti::TreeNode>("s2", recti::Point<int>(150, 50));
        auto root = std::make_shared<recti::TreeNode>("root", recti::Point<int>(100, 50));
        
        sink1->parent = root;
        sink2->parent = root;
        root->left = sink1;
        root->right = sink2;
        
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(50, 50), 1.0),
            recti::Sink("s2", recti::Point<int>(150, 50), 1.0)
        };
        
        recti::SkewAnalysis analysis{
            10.5,    // max_delay
            10.0,    // min_delay
            0.5,     // skew
            {10.0, 10.5},  // sink_delays
            100,     // total_wirelength
            "LinearDelayCalculator"  // delay_model
        };
        
        recti::ClockTreeVisualizer visualizer;
        std::string svg_content = visualizer.visualize_tree(
            root, sinks, "test_with_analysis.svg", 800, 600, &analysis
        );
        
        // Check for analysis box
        CHECK(svg_content.find("Clock Tree Analysis") != std::string::npos);
        CHECK(svg_content.find("LinearDelayCalculator") != std::string::npos);
        CHECK(svg_content.find("Max Delay") != std::string::npos);
        CHECK(svg_content.find("Skew") != std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_with_analysis.svg"));
        
        // Cleanup
        std::filesystem::remove("test_with_analysis.svg");
    }
}

TEST_SUITE("Interactive SVG Tests") {
    TEST_CASE("Create Interactive SVG") {
        auto sink1 = std::make_shared<recti::TreeNode>("sink1", recti::Point<int>(100, 100));
        auto sink2 = std::make_shared<recti::TreeNode>("sink2", recti::Point<int>(200, 200));
        auto root = std::make_shared<recti::TreeNode>("root", recti::Point<int>(150, 150));
        
        sink1->parent = root;
        sink2->parent = root;
        root->left = sink1;
        root->right = sink2;
        
        std::vector<recti::Sink> sinks = {
            recti::Sink("sink1", recti::Point<int>(100, 100), 1.0),
            recti::Sink("sink2", recti::Point<int>(200, 200), 1.0)
        };
        
        recti::SkewAnalysis analysis{
            5.0, 5.0, 0.0, {5.0, 5.0}, 50, "TestCalculator"
        };
        
        std::string svg_content = recti::create_interactive_svg(
            root, sinks, &analysis, "test_interactive.svg"
        );
        
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        CHECK(svg_content.find("interactive") == std::string::npos); // Name doesn't affect content
        
        // Check for custom styling
        CHECK(svg_content.find("#2E7D32") != std::string::npos); // sink_color
        // CHECK(svg_content.find("#1565C0") != std::string::npos); // internal_color
        CHECK(svg_content.find("#C62828") != std::string::npos); // root_color
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_interactive.svg"));
        
        // Cleanup
        // std::filesystem::remove("test_interactive.svg");
    }
    
    TEST_CASE("Create Interactive SVG Without Analysis") {
        auto root = std::make_shared<recti::TreeNode>("root", recti::Point<int>(100, 100));
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(100, 100), 1.0)
        };
        
        std::string svg_content = recti::create_interactive_svg(
            root, sinks, nullptr, "test_interactive_no_analysis.svg"
        );
        
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        
        // Should not have analysis box when analysis is nullptr
        CHECK(svg_content.find("Clock Tree Analysis") == std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_interactive_no_analysis.svg"));
        
        // Cleanup
        // std::filesystem::remove("test_interactive_no_analysis.svg");
    }
}

TEST_SUITE("Comparison Visualization Tests") {
    TEST_CASE("Single Tree Comparison") {
        // Create test tree data
        auto tree1 = std::make_shared<recti::TreeNode>("root1", recti::Point<int>(100, 100));
        std::vector<recti::Sink> sinks1 = {
            recti::Sink("s1", recti::Point<int>(100, 100), 1.0)
        };
        recti::SkewAnalysis analysis1{
            5.0, 5.0, 0.0, {5.0}, 0, "TestModel1"
        };
        
        recti::TreeComparisonData tree_data1{
            tree1, sinks1, analysis1, "Single Tree Test"
        };
        
        std::string svg_content = recti::create_comparison_visualization(
            {tree_data1}, "test_single_comparison.svg", 800, 600
        );
        
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        CHECK(svg_content.find("Single Tree Test") != std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_single_comparison.svg"));
        
        // Cleanup
        // std::filesystem::remove("test_single_comparison.svg");
    }
    
    TEST_CASE("Two Tree Comparison") {
        // Create first test tree
        auto tree1 = std::make_shared<recti::TreeNode>("root1", recti::Point<int>(50, 50));
        std::vector<recti::Sink> sinks1 = {
            recti::Sink("s1", recti::Point<int>(50, 50), 1.0),
            recti::Sink("s2", recti::Point<int>(150, 50), 1.0)
        };
        recti::SkewAnalysis analysis1{
            10.0, 10.0, 0.0, {10.0, 10.0}, 100, "LinearModel"
        };
        
        // Create second test tree
        auto tree2 = std::make_shared<recti::TreeNode>("root2", recti::Point<int>(50, 150));
        std::vector<recti::Sink> sinks2 = {
            recti::Sink("s1", recti::Point<int>(50, 150), 1.0),
            recti::Sink("s2", recti::Point<int>(150, 150), 1.0)
        };
        recti::SkewAnalysis analysis2{
            12.0, 11.0, 1.0, {11.0, 12.0}, 120, "ElmoreModel"
        };
        
        recti::TreeComparisonData tree_data1{tree1, sinks1, analysis1, "Linear Model"};
        recti::TreeComparisonData tree_data2{tree2, sinks2, analysis2, "Elmore Model"};
        
        std::string svg_content = recti::create_comparison_visualization(
            {tree_data1, tree_data2}, "test_two_comparison.svg"
        );
        
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        CHECK(svg_content.find("Linear Model") != std::string::npos);
        CHECK(svg_content.find("Elmore Model") != std::string::npos);
        
        // Should have two columns
        CHECK(svg_content.find("translate(") != std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_two_comparison.svg"));
        
        // Cleanup
        // std::filesystem::remove("test_two_comparison.svg");
    }
    
    TEST_CASE("Delay Model Comparison") {
        // Create test data for delay model comparison
        auto linear_tree = std::make_shared<recti::TreeNode>("linear_root", recti::Point<int>(100, 100));
        auto elmore_tree = std::make_shared<recti::TreeNode>("elmore_root", recti::Point<int>(100, 100));
        
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(50, 50), 1.0),
            recti::Sink("s2", recti::Point<int>(150, 150), 1.0)
        };
        
        recti::SkewAnalysis linear_analysis{
            8.5, 8.5, 0.0, {8.5, 8.5}, 80, "LinearDelayCalculator"
        };
        
        recti::SkewAnalysis elmore_analysis{
            9.2, 9.0, 0.2, {9.0, 9.2}, 95, "ElmoreDelayCalculator"
        };
        
        recti::TreeComparisonData linear_data{linear_tree, sinks, linear_analysis, ""};
        recti::TreeComparisonData elmore_data{elmore_tree, sinks, elmore_analysis, ""};
        
        std::string svg_content = recti::create_delay_model_comparison(
            linear_data, elmore_data, "test_delay_model_comparison.svg"
        );
        
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        CHECK(svg_content.find("Linear Delay Model") != std::string::npos);
        CHECK(svg_content.find("Elmore Delay Model") != std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_delay_model_comparison.svg"));
        
        // Cleanup
        // std::filesystem::remove("test_delay_model_comparison.svg");
    }
    
    TEST_CASE("Empty Comparison Data Throws") {
        CHECK_THROWS_AS(
            recti::create_comparison_visualization({}, "empty_test.svg"),
            std::invalid_argument
        );
    }
}

TEST_SUITE("Integration Tests with DME Algorithm") {
    TEST_CASE("Visualize DME Generated Tree") {
        // Create sinks
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(0, 0), 1.0),
            recti::Sink("s2", recti::Point<int>(100, 0), 1.0),
            recti::Sink("s3", recti::Point<int>(50, 100), 1.0)
        };
        
        // Build clock tree using DME
        auto calculator = std::make_unique<recti::LinearDelayCalculator>(1.0, 1.0);
        recti::DMEAlgorithm dme(sinks, std::move(calculator));
        auto clock_tree = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(clock_tree);
        
        // Visualize the tree
        recti::ClockTreeVisualizer visualizer;
        std::string svg_content = visualizer.visualize_tree(
            clock_tree, sinks, "test_dme_tree.svg", 800, 600, &analysis
        );
        
        // Basic checks
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("<svg") != std::string::npos);
        CHECK(svg_content.find("</svg>") != std::string::npos);
        
        // Should contain all sink names
        CHECK(svg_content.find("s1") != std::string::npos);
        CHECK(svg_content.find("s2") != std::string::npos);
        CHECK(svg_content.find("s3") != std::string::npos);
        
        // Should contain analysis information
        CHECK(svg_content.find("Clock Tree Analysis") != std::string::npos);
        CHECK(svg_content.find("LinearDelayCalculator") != std::string::npos);
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_dme_tree.svg"));
        
        // Cleanup
        // std::filesystem::remove("test_dme_tree.svg");
    }
    
    TEST_CASE("Compare Linear vs Elmore Models") {
        // Create sinks
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(10, 10), 1.0),
            recti::Sink("s2", recti::Point<int>(90, 10), 1.0),
            recti::Sink("s3", recti::Point<int>(50, 90), 1.0)
        };
        
        // Build trees with different delay models
        auto linear_calc = std::make_unique<recti::LinearDelayCalculator>(0.5, 0.2);
        recti::DMEAlgorithm dme_linear(sinks, std::move(linear_calc));
        auto linear_tree = dme_linear.build_clock_tree();
        auto linear_analysis = dme_linear.analyze_skew(linear_tree);
        
        auto elmore_calc = std::make_unique<recti::ElmoreDelayCalculator>(0.1, 0.2);
        recti::DMEAlgorithm dme_elmore(sinks, std::move(elmore_calc));
        auto elmore_tree = dme_elmore.build_clock_tree();
        auto elmore_analysis = dme_elmore.analyze_skew(elmore_tree);
        
        // Create comparison data
        recti::TreeComparisonData linear_data{
            linear_tree, sinks, linear_analysis, "Linear Model"
        };
        recti::TreeComparisonData elmore_data{
            elmore_tree, sinks, elmore_analysis, "Elmore Model"
        };
        
        // Create comparison visualization
        std::string svg_content = recti::create_comparison_visualization(
            {linear_data, elmore_data}, "test_model_comparison.svg"
        );
        
        CHECK_FALSE(svg_content.empty());
        CHECK(svg_content.find("Linear Model") != std::string::npos);
        CHECK(svg_content.find("Elmore Model") != std::string::npos);
        
        // Both trees should have low skew (DME algorithm property)
        CHECK(linear_analysis.skew == doctest::Approx(0.0).epsilon(0.001));
        CHECK(elmore_analysis.skew == doctest::Approx(0.0).epsilon(0.001));
        
        // Verify file was created
        CHECK(std::filesystem::exists("test_model_comparison.svg"));
        
        // Cleanup
        // std::filesystem::remove("test_model_comparison.svg");
    }
}

TEST_SUITE("Edge Cases and Error Handling") {
    TEST_CASE("Visualize Empty Tree") {
        auto empty_tree = std::make_shared<recti::TreeNode>("empty", recti::Point<int>(0, 0));
        std::vector<recti::Sink> empty_sinks;
        
        recti::ClockTreeVisualizer visualizer;
        
        // Should not throw for empty tree
        CHECK_NOTHROW(
            visualizer.visualize_tree(empty_tree, empty_sinks, "test_empty_tree.svg")
        );
        
        // Cleanup if file was created
        if (std::filesystem::exists("test_empty_tree.svg")) {
            std::filesystem::remove("test_empty_tree.svg");
        }
    }
    
    TEST_CASE("Very Large Coordinates") {
        auto root = std::make_shared<recti::TreeNode>("root", recti::Point<int>(10000, 10000));
        auto sink = std::make_shared<recti::TreeNode>("sink", recti::Point<int>(20000, 20000));
        
        root->left = sink;
        sink->parent = root;
        sink->wire_length = 10000;
        
        std::vector<recti::Sink> sinks = {
            recti::Sink("sink", recti::Point<int>(20000, 20000), 1.0)
        };
        
        recti::ClockTreeVisualizer visualizer;
        
        // Should handle large coordinates without issues
        CHECK_NOTHROW(
            visualizer.visualize_tree(root, sinks, "test_large_coords.svg", 800, 600)
        );
        
        // Cleanup if file was created
        if (std::filesystem::exists("test_large_coords.svg")) {
            // std::filesystem::remove("test_large_coords.svg");
        }
    }
    
    TEST_CASE("Duplicate Node Positions") {
        auto sink1 = std::make_shared<recti::TreeNode>("s1", recti::Point<int>(100, 100));
        auto sink2 = std::make_shared<recti::TreeNode>("s2", recti::Point<int>(100, 100)); // Same position
        auto root = std::make_shared<recti::TreeNode>("root", recti::Point<int>(100, 100)); // Also same
        
        root->left = sink1;
        root->right = sink2;
        sink1->parent = root;
        sink2->parent = root;
        
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(100, 100), 1.0),
            recti::Sink("s2", recti::Point<int>(100, 100), 1.0)
        };
        
        recti::ClockTreeVisualizer visualizer;
        
        // Should handle duplicate positions
        CHECK_NOTHROW(
            visualizer.visualize_tree(root, sinks, "test_duplicate_positions.svg")
        );
        
        // Cleanup if file was created
        if (std::filesystem::exists("test_duplicate_positions.svg")) {
            // std::filesystem::remove("test_duplicate_positions.svg");
        }
    }
}

// Example usage demonstration
TEST_CASE("Example Visualization Usage") {
    SUBCASE("Complete Workflow Example") {
        // Step 1: Create clock sinks
        std::vector<recti::Sink> sinks = {
            recti::Sink("s1", recti::Point<int>(-100, 40), 1.0),
            recti::Sink("s2", recti::Point<int>(-60, 60), 1.0),
            recti::Sink("s3", recti::Point<int>(0, 40), 1.0),
            recti::Sink("s4", recti::Point<int>(20, 20), 1.0),
            recti::Sink("s5", recti::Point<int>(-20, -20), 1.0),
            recti::Sink("s6", recti::Point<int>(-30, -50), 1.0),
            recti::Sink("s7", recti::Point<int>(-100, -40), 1.0),
            recti::Sink("s8", recti::Point<int>(-100, 0), 1.0),
        };
        
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

// Test runner main function
// int main(int argc, char** argv) {
//     doctest::Context context;
//     context.applyCommandLine(argc, argv);
    
//     // Create output directory for test files
//     std::filesystem::create_directory("test_output");
    
//     int result = context.run();
    
//     // Cleanup any remaining test files
//     for (const auto& entry : std::filesystem::directory_iterator(".")) {
//         if (entry.path().extension() == ".svg" && 
//             entry.path().string().find("test_") != std::string::npos) {
//             std::filesystem::remove(entry.path());
//         }
//     }
    
//     return result;
// }