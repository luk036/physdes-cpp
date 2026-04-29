#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

// Include the DME algorithm headers
#include "dme_algorithm.hpp"

namespace recti {

    /**
     * @brief Visualizes clock trees in SVG format
     */
    class ClockTreeVisualizer {
      private:
        int margin;                  ///< Margin around the SVG canvas
        int node_radius;             ///< Radius of nodes in the visualization
        int wire_width;              ///< Width of wires connecting nodes
        std::string sink_color;      ///< Color for sink nodes
        std::string internal_color;  ///< Color for internal nodes
        std::string root_color;      ///< Color for root nodes
        std::string wire_color;      ///< Color for wires
        std::string text_color;      ///< Color for text labels

      public:
        /**
         * @brief Initialize the visualizer with styling parameters
         * @param margin Margin around the SVG canvas
         * @param node_radius Radius of nodes in the visualization
         * @param wire_width Width of wires connecting nodes
         * @param sink_color Color for sink nodes
         * @param internal_color Color for internal nodes
         * @param root_color Color for root nodes
         * @param wire_color Color for wires
         * @param text_color Color for text labels
         */
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

        /**
         * @brief Create an SVG visualization of the clock tree
         * @param[in] root The root node of the clock tree
         * @param[in] sinks The list of sinks in the clock tree
         * @param[in] filename The output filename for the SVG file
         * @param[in] width The width of the SVG canvas
         * @param[in] height The height of the SVG canvas
         * @param[in] analysis Optional pointer to skew analysis results to display
         */
        std::string visualize_tree(const std::shared_ptr<TreeNode>& root,
                                 const std::vector<Sink>& sinks,
                                 const std::string& filename = "clock_tree.svg", int width = 800,
                                 int height = 600, const SkewAnalysis* analysis = nullptr);

      private:
        std::vector<std::shared_ptr<TreeNode>> collect_all_nodes(
            const std::shared_ptr<TreeNode>& root);

        std::tuple<double, double, double, double> calculate_bounds(
            const std::vector<std::shared_ptr<TreeNode>>& nodes, const std::vector<Sink>& sinks);

        std::vector<std::string> draw_wires(
            const std::shared_ptr<TreeNode>& root,
            const std::function<std::pair<double, double>(double, double)>& scale_coord);

        std::vector<std::string> draw_nodes(
            const std::shared_ptr<TreeNode>& root, const std::vector<Sink>& sinks,
            const std::function<std::pair<double, double>(double, double)>& scale_coord);

        std::vector<std::string> create_analysis_box(const SkewAnalysis& analysis,
                                                 int width);
    };

    /**
     * @brief Create an interactive SVG with additional information and styling
     * @param[in] root The root node of the clock tree
     * @param[in] sinks The list of sinks in the clock tree
     * @param[in] analysis Optional pointer to skew analysis results to display
     * @param[in] filename The output filename for the SVG file
     * @param[in] width The width of the SVG canvas
     * @param[in] height The height of the SVG canvas
     */
    std::string create_interactive_svg(const std::shared_ptr<TreeNode>& root,
                                           const std::vector<Sink>& sinks,
                                           const SkewAnalysis* analysis = nullptr,
                                           const std::string& filename
                                           = "clock_tree_interactive.svg",
                                           int width = 1000, int height = 700);

    /**
     * @brief Tree data structure for comparison visualization
     */
    struct TreeComparisonData {
        std::shared_ptr<TreeNode> tree;
        std::vector<Sink> sinks;
        SkewAnalysis analysis;
        std::string title;
    };

    /**
     * @brief Create a comparison visualization of multiple clock trees
     * @param[in] trees_data The data for multiple trees to compare
     * @param[in] filename The output filename for the SVG file
     * @param[in] width The width of the SVG canvas
     * @param[in] height The height of the SVG canvas
     */
    std::string create_comparison_visualization(
        const std::vector<TreeComparisonData>& trees_data,
        const std::string& filename = "clock_tree_comparison.svg", int width = 1200,
        int height = 800);

/**
     * @brief Create a specialized comparison between linear and Elmore delay models
     * @param[in] linear_tree_data The tree data using the linear delay model
     * @param[in] elmore_tree_data The tree data using the Elmore delay model
     * @param[in] filename The output filename for the SVG file
     */
    std::string create_delay_model_comparison(const TreeComparisonData& linear_tree_data,
                                                const TreeComparisonData& elmore_tree_data,
                                                const std::string& filename
                                                = "delay_model_comparison.svg");

    /**
     * @brief Example function demonstrating clock tree visualization with different delay models
     */
    void visualize_example_tree();

}  // namespace recti