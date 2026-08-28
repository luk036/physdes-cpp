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
     *
     * Configured through the nested `Builder` (Builder pattern) instead of a
     * telescoping constructor. Example:
     *
     * @code{.cpp}
     * auto viz = ClockTreeVisualizer::Builder()
     *                .node_radius(10)
     *                .wire_width(3)
     *                .sink_color("#2E7D32")
     *                .build();
     * @endcode
     */
    class ClockTreeVisualizer {
      public:
        /**
         * @brief Fluent builder for ClockTreeVisualizer configuration.
         *
         * Builder pattern: collects appearance options via chained setters and
         * produces a ClockTreeVisualizer with `build()`.
         */
        class Builder {
          public:
            Builder() = default;

            /** @brief Sets the drawing margin. */
            auto margin(int value) -> Builder& {
                _margin = value;
                return *this;
            }

            /** @brief Sets the base radius of node circles. */
            auto node_radius(int value) -> Builder& {
                _node_radius = value;
                return *this;
            }

            /** @brief Sets the wire stroke width. */
            auto wire_width(int value) -> Builder& {
                _wire_width = value;
                return *this;
            }

            /** @brief Sets the sink node color. */
            auto sink_color(std::string value) -> Builder& {
                _sink_color = std::move(value);
                return *this;
            }

            /** @brief Sets the internal node color. */
            auto internal_color(std::string value) -> Builder& {
                _internal_color = std::move(value);
                return *this;
            }

            /** @brief Sets the root node color. */
            auto root_color(std::string value) -> Builder& {
                _root_color = std::move(value);
                return *this;
            }

            /** @brief Sets the wire color. */
            auto wire_color(std::string value) -> Builder& {
                _wire_color = std::move(value);
                return *this;
            }

            /** @brief Sets the text color. */
            auto text_color(std::string value) -> Builder& {
                _text_color = std::move(value);
                return *this;
            }

            /** @brief Builds the configured ClockTreeVisualizer. */
            auto build() const -> ClockTreeVisualizer {
                return ClockTreeVisualizer(_margin, _node_radius, _wire_width, _sink_color,
                                           _internal_color, _root_color, _wire_color, _text_color);
            }

          private:
            int _margin = 50;
            int _node_radius = 8;
            int _wire_width = 2;
            std::string _sink_color = "#4CAF50";
            std::string _internal_color = "#2196F3";
            std::string _root_color = "#F44336";
            std::string _wire_color = "#666666";
            std::string _text_color = "#333333";
        };

        /**
         * @brief Constructs a ClockTreeVisualizer with default appearance.
         */
        ClockTreeVisualizer() = default;

        std::string visualize_tree(const Tree& tree, NodeIdx root, const std::vector<Sink>& sinks,
                                   const std::string& filename = "clock_tree.svg", int width = 800,
                                   int height = 600, const SkewAnalysis* analysis = nullptr);

      private:
        friend class Builder;

        ClockTreeVisualizer(int margin, int node_radius, int wire_width,
                            const std::string& sink_color, const std::string& internal_color,
                            const std::string& root_color, const std::string& wire_color,
                            const std::string& text_color)
            : margin(margin),
              node_radius(node_radius),
              wire_width(wire_width),
              sink_color(sink_color),
              internal_color(internal_color),
              root_color(root_color),
              wire_color(wire_color),
              text_color(text_color) {}

        int margin = 50;
        int node_radius = 8;
        int wire_width = 2;
        std::string sink_color = "#4CAF50";
        std::string internal_color = "#2196F3";
        std::string root_color = "#F44336";
        std::string wire_color = "#666666";
        std::string text_color = "#333333";

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
