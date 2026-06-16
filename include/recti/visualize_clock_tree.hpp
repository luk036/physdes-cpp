/** @file visualize_clock_tree.hpp
 *  @brief SVG visualization utilities for clock trees.
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "dme_algorithm.hpp"

namespace recti {

    /**
     * @brief Visualizes clock trees in SVG format
     */
    class ClockTreeVisualizer {
      private:
        int margin;
        int node_radius;
        int wire_width;
        std::string sink_color;
        std::string internal_color;
        std::string root_color;
        std::string wire_color;
        std::string text_color;

      public:
        ClockTreeVisualizer(int margin = 50, int node_radius = 8, int wire_width = 2,
                            const std::string& sink_color = "#4CAF50",
                            const std::string& internal_color = "#2196F3",
                            const std::string& root_color = "#F44336",
                            const std::string& wire_color = "#666666",
                            const std::string& text_color = "#333333")
            : margin(margin),
              node_radius(node_radius),
              wire_width(wire_width),
              sink_color(sink_color),
              internal_color(internal_color),
              root_color(root_color),
              wire_color(wire_color),
              text_color(text_color) {}

        std::string visualize_tree(const Tree& tree, NodeIdx root, const std::vector<Sink>& sinks,
                                    const std::string& filename = "clock_tree.svg", int width = 800,
                                    int height = 600, const SkewAnalysis* analysis = nullptr);

      private:
        std::vector<NodeIdx> collect_all_nodes(const Tree& tree, NodeIdx root);

        std::tuple<double, double, double, double> calculate_bounds(
            const std::vector<NodeIdx>& node_indices, const std::vector<Sink>& sinks,
            const Tree& tree);

        std::vector<std::string> draw_wires(
            NodeIdx root, const Tree& tree,
            const std::function<std::pair<double, double>(double, double)>& scale_coord);

        std::vector<std::string> draw_nodes(
            NodeIdx root, const Tree& tree, const std::vector<Sink>& sinks,
            const std::function<std::pair<double, double>(double, double)>& scale_coord);

        std::vector<std::string> create_analysis_box(const SkewAnalysis& analysis, int width);
    };

    std::string create_interactive_svg(const Tree& tree, NodeIdx root,
                                        const std::vector<Sink>& sinks,
                                        const SkewAnalysis* analysis = nullptr,
                                        const std::string& filename = "clock_tree_interactive.svg",
                                        int width = 1000, int height = 700);

    struct TreeComparisonData {
        const Tree* tree;
        NodeIdx root;
        std::vector<Sink> sinks;
        SkewAnalysis analysis;
        std::string title;
    };

    std::string create_comparison_visualization(const std::vector<TreeComparisonData>& trees_data,
                                                 const std::string& filename
                                                 = "clock_tree_comparison.svg",
                                                 int width = 1200, int height = 800);

    std::string create_delay_model_comparison(const TreeComparisonData& linear_tree_data,
                                               const TreeComparisonData& elmore_tree_data,
                                               const std::string& filename
                                               = "delay_model_comparison.svg");

    void visualize_example_tree();

}  // namespace recti
