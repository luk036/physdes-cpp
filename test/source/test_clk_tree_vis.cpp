#include <doctest/doctest.h>

#include <filesystem>
#include <memory>
#include <recti/visualize_clock_tree.hpp>
#include <string>
#include <vector>

// Helper: build a simple two-sink tree in a Tree arena and return the root.
// Sinks: s1=(50,50), s2=(150,150), root=(100,100).
static recti::NodeIdx build_two_sink_tree(recti::Tree& tree) {
    using namespace recti;
    NodeIdx s1 = tree.add(TreeNode("s1", Point<int>(50, 50)));
    NodeIdx s2 = tree.add(TreeNode("s2", Point<int>(150, 150)));
    NodeIdx root = tree.add(TreeNode("root", Point<int>(100, 100)));

    tree.get_mut(s1).parent = root;
    tree.get_mut(s2).parent = root;
    tree.get_mut(root).left = s1;
    tree.get_mut(root).right = s2;

    tree.get_mut(s1).wire_length = 10;
    tree.get_mut(s2).wire_length = 10;
    tree.get_mut(s1).delay = 5.0;
    tree.get_mut(s2).delay = 5.0;
    tree.get_mut(s1).capacitance = 1.0;
    tree.get_mut(s2).capacitance = 1.0;
    tree.get_mut(root).capacitance = 2.0;
    return root;
}

TEST_SUITE("ClockTreeVisualizer Tests") {
    TEST_CASE("Single Sink Visualization") {
        using namespace recti;
        Tree tree;
        NodeIdx sink = tree.add(TreeNode("s1", Point<int>(100, 100)));
        tree.get_mut(sink).capacitance = 1.5;
        tree.get_mut(sink).delay = 0.0;

        std::vector<Sink> sinks = {Sink("s1", Point<int>(100, 100), 1.5)};
        ClockTreeVisualizer visualizer;
        std::string svg = visualizer.visualize_tree(tree, sink, sinks,
                                                    "test_single_sink.svg", 400, 300);
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("<svg"), std::string::npos);
        CHECK_NE(svg.find("</svg>"), std::string::npos);
        CHECK_NE(svg.find("s1"), std::string::npos);
        CHECK_NE(svg.find("circle"), std::string::npos);
        CHECK(std::filesystem::exists("test_single_sink.svg"));
        std::filesystem::remove("test_single_sink.svg");
    }

    TEST_CASE("Two Sink Tree Visualization") {
        using namespace recti;
        Tree tree;
        NodeIdx root = build_two_sink_tree(tree);

        std::vector<Sink> sinks = {Sink("s1", Point<int>(50, 50), 1.0),
                                   Sink("s2", Point<int>(150, 150), 1.0)};
        ClockTreeVisualizer visualizer;
        std::string svg = visualizer.visualize_tree(tree, root, sinks,
                                                    "test_two_sinks.svg", 600, 400);

        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("<svg"), std::string::npos);
        CHECK_NE(svg.find("</svg>"), std::string::npos);
        CHECK_NE(svg.find("s1"), std::string::npos);
        CHECK_NE(svg.find("s2"), std::string::npos);
        CHECK_NE(svg.find("root"), std::string::npos);
        CHECK_NE(svg.find("line"), std::string::npos);
        CHECK(std::filesystem::exists("test_two_sinks.svg"));
        std::filesystem::remove("test_two_sinks.svg");
    }

    TEST_CASE("Visualization with Analysis") {
        using namespace recti;
        Tree tree;
        NodeIdx s1 = tree.add(TreeNode("s1", Point<int>(50, 50)));
        NodeIdx s2 = tree.add(TreeNode("s2", Point<int>(150, 50)));
        NodeIdx root = tree.add(TreeNode("root", Point<int>(100, 50)));
        tree.get_mut(s1).parent = root;
        tree.get_mut(s2).parent = root;
        tree.get_mut(root).left = s1;
        tree.get_mut(root).right = s2;

        std::vector<Sink> sinks = {Sink("s1", Point<int>(50, 50), 1.0),
                                   Sink("s2", Point<int>(150, 50), 1.0)};
        SkewAnalysis analysis{.max_delay = 10.5,
                              .min_delay = 10.0,
                              .skew = 0.5,
                              .sink_delays = {10.0, 10.5},
                              .total_wirelength = 100,
                              .delay_model = "LinearDelayCalculator"};

        ClockTreeVisualizer visualizer;
        std::string svg = visualizer.visualize_tree(tree, root, sinks,
                                                    "test_with_analysis.svg",
                                                    800, 600, &analysis);
        CHECK_NE(svg.find("Clock Tree Analysis"), std::string::npos);
        CHECK_NE(svg.find("LinearDelayCalculator"), std::string::npos);
        CHECK_NE(svg.find("Max Delay"), std::string::npos);
        CHECK_NE(svg.find("Skew"), std::string::npos);
        CHECK(std::filesystem::exists("test_with_analysis.svg"));
        std::filesystem::remove("test_with_analysis.svg");
    }
}

TEST_SUITE("Interactive SVG Tests") {
    TEST_CASE("Create Interactive SVG") {
        using namespace recti;
        Tree tree;
        NodeIdx s1 = tree.add(TreeNode("sink1", Point<int>(100, 100)));
        NodeIdx s2 = tree.add(TreeNode("sink2", Point<int>(200, 200)));
        NodeIdx root = tree.add(TreeNode("root", Point<int>(150, 150)));
        tree.get_mut(s1).parent = root;
        tree.get_mut(s2).parent = root;
        tree.get_mut(root).left = s1;
        tree.get_mut(root).right = s2;

        std::vector<Sink> sinks = {Sink("sink1", Point<int>(100, 100), 1.0),
                                   Sink("sink2", Point<int>(200, 200), 1.0)};
        SkewAnalysis analysis{.max_delay = 5.0, .min_delay = 5.0, .skew = 0.0,
                              .sink_delays = {5.0, 5.0}, .total_wirelength = 50,
                              .delay_model = "TestCalculator"};
        std::string svg = create_interactive_svg(tree, root, sinks, &analysis,
                                                 "test_interactive.svg");
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("<svg"), std::string::npos);
        CHECK_NE(svg.find("#2E7D32"), std::string::npos);
        CHECK_NE(svg.find("#C62828"), std::string::npos);
        CHECK(std::filesystem::exists("test_interactive.svg"));
        std::filesystem::remove("test_interactive.svg");
    }

    TEST_CASE("Create Interactive SVG Without Analysis") {
        using namespace recti;
        Tree tree;
        NodeIdx root = tree.add(TreeNode("root", Point<int>(100, 100)));
        std::vector<Sink> sinks = {Sink("s1", Point<int>(100, 100), 1.0)};
        std::string svg = create_interactive_svg(tree, root, sinks, nullptr,
                                                 "test_interactive_no_analysis.svg");
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("<svg"), std::string::npos);
        CHECK_EQ(svg.find("Clock Tree Analysis"), std::string::npos);
        CHECK(std::filesystem::exists("test_interactive_no_analysis.svg"));
        std::filesystem::remove("test_interactive_no_analysis.svg");
    }
}

TEST_SUITE("Comparison Visualization Tests") {
    TEST_CASE("Single Tree Comparison") {
        using namespace recti;
        Tree tree;
        NodeIdx root = tree.add(TreeNode("root1", Point<int>(100, 100)));
        std::vector<Sink> sinks1 = {Sink("s1", Point<int>(100, 100), 1.0)};
        SkewAnalysis analysis1{.max_delay = 5.0, .min_delay = 5.0, .skew = 0.0,
                               .sink_delays = {5.0}, .total_wirelength = 0,
                               .delay_model = "TestModel1"};
        TreeComparisonData data{.tree = &tree, .root = root, .sinks = sinks1,
                                .analysis = analysis1, .title = "Single Tree Test"};
        std::string svg = create_comparison_visualization({data}, "test_single_comparison.svg",
                                                          800, 600);
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("<svg"), std::string::npos);
        CHECK_NE(svg.find("Single Tree Test"), std::string::npos);
        CHECK(std::filesystem::exists("test_single_comparison.svg"));
        std::filesystem::remove("test_single_comparison.svg");
    }

    TEST_CASE("Two Tree Comparison") {
        using namespace recti;
        Tree tree1, tree2;
        NodeIdx r1 = tree1.add(TreeNode("root1", Point<int>(50, 50)));
        NodeIdx r2 = tree2.add(TreeNode("root2", Point<int>(50, 150)));
        std::vector<Sink> sinks1 = {Sink("s1", Point<int>(50, 50), 1.0),
                                    Sink("s2", Point<int>(150, 50), 1.0)};
        std::vector<Sink> sinks2 = {Sink("s1", Point<int>(50, 150), 1.0),
                                    Sink("s2", Point<int>(150, 150), 1.0)};
        SkewAnalysis a1{.max_delay = 10.0, .min_delay = 10.0, .skew = 0.0,
                        .sink_delays = {10.0, 10.0}, .total_wirelength = 100,
                        .delay_model = "LinearModel"};
        SkewAnalysis a2{.max_delay = 12.0, .min_delay = 11.0, .skew = 1.0,
                        .sink_delays = {11.0, 12.0}, .total_wirelength = 120,
                        .delay_model = "ElmoreModel"};
        TreeComparisonData d1{.tree = &tree1, .root = r1, .sinks = sinks1,
                              .analysis = a1, .title = "Linear Model"};
        TreeComparisonData d2{.tree = &tree2, .root = r2, .sinks = sinks2,
                              .analysis = a2, .title = "Elmore Model"};
        std::string svg = create_comparison_visualization({d1, d2},
                                                          "test_two_comparison.svg");
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("Linear Model"), std::string::npos);
        CHECK_NE(svg.find("Elmore Model"), std::string::npos);
        CHECK_NE(svg.find("translate("), std::string::npos);
        CHECK(std::filesystem::exists("test_two_comparison.svg"));
        std::filesystem::remove("test_two_comparison.svg");
    }

    TEST_CASE("Delay Model Comparison") {
        using namespace recti;
        Tree linear_tree, elmore_tree;
        NodeIdx lr = linear_tree.add(TreeNode("linear_root", Point<int>(100, 100)));
        NodeIdx er = elmore_tree.add(TreeNode("elmore_root", Point<int>(100, 100)));
        std::vector<Sink> sinks = {Sink("s1", Point<int>(50, 50), 1.0),
                                   Sink("s2", Point<int>(150, 150), 1.0)};
        SkewAnalysis la{.max_delay = 8.5, .min_delay = 8.5, .skew = 0.0,
                        .sink_delays = {8.5, 8.5}, .total_wirelength = 80,
                        .delay_model = "LinearDelayCalculator"};
        SkewAnalysis ea{.max_delay = 9.2, .min_delay = 9.0, .skew = 0.2,
                        .sink_delays = {9.0, 9.2}, .total_wirelength = 95,
                        .delay_model = "ElmoreDelayCalculator"};
        TreeComparisonData ld{.tree = &linear_tree, .root = lr, .sinks = sinks,
                              .analysis = la, .title = ""};
        TreeComparisonData ed{.tree = &elmore_tree, .root = er, .sinks = sinks,
                              .analysis = ea, .title = ""};
        std::string svg = create_delay_model_comparison(ld, ed,
                                                        "test_delay_model_comparison.svg");
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("Linear Delay Model"), std::string::npos);
        CHECK_NE(svg.find("Elmore Delay Model"), std::string::npos);
        CHECK(std::filesystem::exists("test_delay_model_comparison.svg"));
        std::filesystem::remove("test_delay_model_comparison.svg");
    }

    TEST_CASE("Empty Comparison Data Throws") {
        CHECK_THROWS_AS(recti::create_comparison_visualization({}, "empty_test.svg"),
                        std::invalid_argument);
    }
}

TEST_SUITE("Integration Tests with DME Algorithm") {
    TEST_CASE("Visualize DME Generated Tree") {
        using namespace recti;
        std::vector<Sink> sinks = {Sink("s1", Point<int>(0, 0), 1.0),
                                   Sink("s2", Point<int>(100, 0), 1.0),
                                   Sink("s3", Point<int>(50, 100), 1.0)};
        auto calculator = std::make_unique<LinearDelayCalculator>(1.0, 1.0);
        DMEAlgorithm dme(sinks, std::move(calculator));
        NodeIdx root = dme.build_clock_tree();
        auto analysis = dme.analyze_skew(root);

        ClockTreeVisualizer visualizer;
        std::string svg = visualizer.visualize_tree(dme.get_tree(), root, sinks,
                                                    "test_dme_tree.svg", 800, 600, &analysis);
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("<svg"), std::string::npos);
        CHECK_NE(svg.find("</svg>"), std::string::npos);
        CHECK_NE(svg.find("s1"), std::string::npos);
        CHECK_NE(svg.find("s2"), std::string::npos);
        CHECK_NE(svg.find("s3"), std::string::npos);
        CHECK_NE(svg.find("Clock Tree Analysis"), std::string::npos);
        CHECK_NE(svg.find("LinearDelayCalculator"), std::string::npos);
        CHECK(std::filesystem::exists("test_dme_tree.svg"));
        std::filesystem::remove("test_dme_tree.svg");
    }

    TEST_CASE("Compare Linear vs Elmore Models") {
        using namespace recti;
        std::vector<Sink> sinks = {Sink("s1", Point<int>(10, 10), 1.0),
                                   Sink("s2", Point<int>(90, 10), 1.0),
                                   Sink("s3", Point<int>(50, 90), 1.0)};

        auto lc = std::make_unique<LinearDelayCalculator>(0.5, 0.2);
        DMEAlgorithm dme_linear(sinks, std::move(lc));
        NodeIdx rl = dme_linear.build_clock_tree();
        auto al = dme_linear.analyze_skew(rl);

        auto ec = std::make_unique<ElmoreDelayCalculator>(0.1, 0.2);
        DMEAlgorithm dme_elmore(sinks, std::move(ec));
        NodeIdx re = dme_elmore.build_clock_tree();
        auto ae = dme_elmore.analyze_skew(re);

        TreeComparisonData ld{.tree = &dme_linear.get_tree(), .root = rl,
                              .sinks = sinks, .analysis = al, .title = "Linear Model"};
        TreeComparisonData ed{.tree = &dme_elmore.get_tree(), .root = re,
                              .sinks = sinks, .analysis = ae, .title = "Elmore Model"};
        std::string svg = create_comparison_visualization({ld, ed},
                                                          "test_model_comparison.svg");
        CHECK_FALSE(svg.empty());
        CHECK_NE(svg.find("Linear Model"), std::string::npos);
        CHECK_NE(svg.find("Elmore Model"), std::string::npos);
        CHECK_EQ(al.skew, doctest::Approx(0.0).epsilon(0.001));
        CHECK_EQ(ae.skew, doctest::Approx(0.0).epsilon(0.001));
        CHECK(std::filesystem::exists("test_model_comparison.svg"));
        std::filesystem::remove("test_model_comparison.svg");
    }
}

TEST_SUITE("Edge Cases and Error Handling") {
    TEST_CASE("Visualize Single Node Tree") {
        using namespace recti;
        Tree tree;
        NodeIdx root = tree.add(TreeNode("single", Point<int>(0, 0)));
        std::vector<Sink> empty_sinks;
        ClockTreeVisualizer visualizer;
        CHECK_NOTHROW(visualizer.visualize_tree(tree, root, empty_sinks,
                                                "test_single_node.svg"));
        if (std::filesystem::exists("test_single_node.svg")) {
            std::filesystem::remove("test_single_node.svg");
        }
    }

    TEST_CASE("Very Large Coordinates") {
        using namespace recti;
        Tree tree;
        NodeIdx root = tree.add(TreeNode("root", Point<int>(10000, 10000)));
        NodeIdx sink = tree.add(TreeNode("sink", Point<int>(20000, 20000)));
        tree.get_mut(root).left = sink;
        tree.get_mut(sink).parent = root;
        tree.get_mut(sink).wire_length = 10000;

        std::vector<Sink> sinks = {Sink("sink", Point<int>(20000, 20000), 1.0)};
        ClockTreeVisualizer visualizer;
        CHECK_NOTHROW(visualizer.visualize_tree(tree, root, sinks,
                                                "test_large_coords.svg", 800, 600));
        if (std::filesystem::exists("test_large_coords.svg")) {
            std::filesystem::remove("test_large_coords.svg");
        }
    }

    TEST_CASE("Duplicate Node Positions") {
        using namespace recti;
        Tree tree;
        NodeIdx s1 = tree.add(TreeNode("s1", Point<int>(100, 100)));
        NodeIdx s2 = tree.add(TreeNode("s2", Point<int>(100, 100)));
        NodeIdx root = tree.add(TreeNode("root", Point<int>(100, 100)));
        tree.get_mut(root).left = s1;
        tree.get_mut(root).right = s2;
        tree.get_mut(s1).parent = root;
        tree.get_mut(s2).parent = root;

        std::vector<Sink> sinks = {Sink("s1", Point<int>(100, 100), 1.0),
                                   Sink("s2", Point<int>(100, 100), 1.0)};
        ClockTreeVisualizer visualizer;
        CHECK_NOTHROW(visualizer.visualize_tree(tree, root, sinks,
                                                "test_duplicate_positions.svg"));
        if (std::filesystem::exists("test_duplicate_positions.svg")) {
            std::filesystem::remove("test_duplicate_positions.svg");
        }
    }
}

TEST_CASE("Example Visualization Usage") {
    using namespace recti;
    SUBCASE("Complete Workflow Example") {
        std::vector<Sink> sinks = {
            Sink("s1", Point<int>(-100, 40), 1.0), Sink("s2", Point<int>(-60, 60), 1.0),
            Sink("s3", Point<int>(0, 40), 1.0),    Sink("s4", Point<int>(20, 20), 1.0),
            Sink("s5", Point<int>(-20, -20), 1.0), Sink("s6", Point<int>(-30, -50), 1.0),
            Sink("s7", Point<int>(-100, -40), 1.0), Sink("s8", Point<int>(-100, 0), 1.0),
        };

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

        std::filesystem::remove("basic_clock_tree.svg");
        std::filesystem::remove("interactive_clock_tree.svg");
    }
}
