#pragma once

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "dme_algorithm.hpp"
#include "point.hpp"

namespace recti {

    /**
     * @brief Hash function for std::pair<int, int>.
     */
    struct PairHash {
        template <typename T1, typename T2>
        std::size_t operator()(const std::pair<T1, T2>& p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };

    /**
     * @brief Utility function to split a string by delimiter.
     */
    inline auto split_string(const std::string& str, char delimiter) -> std::vector<std::string> {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;
        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }
        return result;
    }

    /**
     * @brief Utility function to join strings with a delimiter.
     */
    inline auto join_strings(const std::vector<std::string>& strings, const std::string& delimiter)
        -> std::string {
        std::string result;
        for (size_t i = 0; i < strings.size(); ++i) {
            result += strings[i];
            if (i < strings.size() - 1) result += delimiter;
        }
        return result;
    }

    /**
     * @brief Visualizes clock trees in SVG format.
     */
    class ClockTreeVisualizer {
      public:
        /**
         * @brief Construct a ClockTreeVisualizer with styling parameters.
         *
         * @param margin Margin around the drawing.
         * @param node_radius Radius of node circles.
         * @param wire_width Width of wire lines.
         * @param sink_color Color for sink nodes.
         * @param internal_color Color for internal nodes.
         * @param root_color Color for root node.
         * @param wire_color Color for wires.
         * @param text_color Color for text labels.
         */
        ClockTreeVisualizer(int margin = 50, int node_radius = 8, int wire_width = 2,
                            const std::string& sink_color = "#4CAF50",
                            const std::string& internal_color = "#2196F3",
                            const std::string& root_color = "#F44336",
                            const std::string& wire_color = "#666666",
                            const std::string& text_color = "#333333")
            : margin_(margin),
              node_radius_(node_radius),
              wire_width_(wire_width),
              sink_color_(sink_color),
              internal_color_(internal_color),
              root_color_(root_color),
              wire_color_(wire_color),
              text_color_(text_color) {}

        /**
         * @brief Get the margin.
         */
        auto margin() const -> int { return margin_; }

        /**
         * @brief Get the node radius.
         */
        auto node_radius() const -> int { return node_radius_; }

        /**
         * @brief Create an SVG visualization of the clock tree.
         *
         * @param root Root node of the clock tree.
         * @param sinks List of original sink objects.
         * @param filename Output filename (empty string to skip saving).
         * @param width SVG width.
         * @param height SVG height.
         * @param analysis Optional analysis results from DME algorithm.
         * @return SVG string content.
         */
        auto visualize_tree(
            const TreeNode* root, const std::vector<Sink>& sinks,
            const std::string& filename = "clock_tree.svg", int width = 800, int height = 600,
            const std::optional<std::unordered_map<
                std::string, std::variant<float, std::string, std::vector<float>>>>& analysis
            = std::nullopt) -> std::string {
            // Collect all nodes and calculate bounds
            auto all_nodes = collect_all_nodes(root);
            auto [min_x, min_y, max_x, max_y] = calculate_bounds(all_nodes, sinks);

            // Scale coordinates to fit SVG canvas
            float scale_x = (max_x > min_x)
                                ? static_cast<float>(width - 2 * margin_) / (max_x - min_x)
                                : 1.0f;
            float scale_y = (max_y > min_y)
                                ? static_cast<float>(height - 2 * margin_) / (max_y - min_y)
                                : 1.0f;
            float scale = std::min(scale_x, scale_y);

            auto scale_coord
                = [this, min_x, min_y, scale](float x, float y) -> std::pair<float, float> {
                return {(x - min_x) * scale + static_cast<float>(margin_),
                        (y - min_y) * scale + static_cast<float>(margin_)};
            };

            // Create SVG content
            std::vector<std::string> svg_content
                = {"<svg width=\"" + std::to_string(width) + "\" height=\"" + std::to_string(height)
                       + "\" xmlns=\"http://www.w3.org/2000/svg\">",
                   "<style>",
                   "  .node-label { font: 10px sans-serif; fill: " + text_color_ + "; }",
                   "  .delay-label { font: 8px sans-serif; fill: #666; }",
                   "  .wire-label { font: 9px sans-serif; fill: #444; }",
                   "  .analysis-label { font: 12px sans-serif; fill: #333; }",
                   "</style>",
                   "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>",
                   "<g class=\"clock-tree\">"};

            // Draw wires
            auto wires = draw_wires(root, scale_coord);
            svg_content.insert(svg_content.end(), wires.begin(), wires.end());

            // Draw nodes
            auto nodes = draw_nodes(root, sinks, scale_coord);
            svg_content.insert(svg_content.end(), nodes.begin(), nodes.end());

            // Add analysis information
            if (analysis) {
                auto analysis_box = create_analysis_box(*analysis);
                svg_content.insert(svg_content.end(), analysis_box.begin(), analysis_box.end());
            }

            // Close SVG
            svg_content.push_back("</g>");
            svg_content.push_back("</svg>");

            // Join content
            std::string svg_string;
            for (const auto& line : svg_content) {
                svg_string += line + "\n";
            }

            // Save to file
            if (!filename.empty()) {
                std::ofstream file(filename);
                file << svg_string;
                file.close();
                std::cout << "Clock tree visualization saved to " << filename << std::endl;
            }

            return svg_string;
        }

      private:
        /**
         * @brief Collect all nodes in the tree.
         */
        auto collect_all_nodes(const TreeNode* root) -> std::vector<const TreeNode*> {
            std::vector<const TreeNode*> nodes;
            std::function<void(const TreeNode*)> collect = [&](const TreeNode* node) {
                if (node) {
                    nodes.push_back(node);
                    if (node->left()) collect(node->left().value());
                    if (node->right()) collect(node->right().value());
                }
            };
            collect(root);
            return nodes;
        }

        /**
         * @brief Calculate the bounding box of all nodes and sinks.
         */
        auto calculate_bounds(const std::vector<const TreeNode*>& nodes,
                              const std::vector<Sink>& sinks)
            -> std::tuple<float, float, float, float> {
            std::vector<std::pair<float, float>> all_points;
            for (const auto* node : nodes) {
                all_points.emplace_back(static_cast<float>(node->position().xcoord()),
                                        static_cast<float>(node->position().ycoord()));
            }
            for (const auto& sink : sinks) {
                all_points.emplace_back(static_cast<float>(sink.position().xcoord()),
                                        static_cast<float>(sink.position().ycoord()));
            }

            if (all_points.empty()) {
                return {0.0f, 0.0f, 100.0f, 100.0f};
            }

            float min_x
                = std::min_element(all_points.begin(), all_points.end(),
                                   [](const auto& a, const auto& b) { return a.first < b.first; })
                      ->first;
            float max_x
                = std::max_element(all_points.begin(), all_points.end(),
                                   [](const auto& a, const auto& b) { return a.first < b.first; })
                      ->first;
            float min_y
                = std::min_element(all_points.begin(), all_points.end(),
                                   [](const auto& a, const auto& b) { return a.second < b.second; })
                      ->second;
            float max_y
                = std::max_element(all_points.begin(), all_points.end(),
                                   [](const auto& a, const auto& b) { return a.second < b.second; })
                      ->second;

            float padding = std::max({(max_x - min_x) * 0.1f, (max_y - min_y) * 0.1f, 10.0f});
            return {min_x - padding, min_y - padding, max_x + padding, max_y + padding};
        }

        /**
         * @brief Draw all wires in the clock tree.
         */
        auto draw_wires(const TreeNode* root,
                        const std::function<std::pair<float, float>(float, float)>& scale_coord)
            -> std::vector<std::string> {
            std::vector<std::string> svg_elements;
            std::function<void(const TreeNode*)> draw_wires_recursive = [&](const TreeNode* node) {
                if (!node) return;
                if (node->parent()) {
                    auto [x1, y1] = scale_coord(
                        static_cast<float>(node->parent().value()->position().xcoord()),
                        static_cast<float>(node->parent().value()->position().ycoord()));
                    auto [x2, y2] = scale_coord(static_cast<float>(node->position().xcoord()),
                                                static_cast<float>(node->position().ycoord()));

                    svg_elements.push_back(
                        "<line x1=\"" + std::to_string(x1) + "\" y1=\"" + std::to_string(y1)
                        + "\" x2=\"" + std::to_string(x2) + "\" y2=\"" + std::to_string(y2)
                        + "\" stroke=\"" + wire_color_ + "\" stroke-width=\""
                        + std::to_string(wire_width_) + "\" stroke-linecap=\"round\"/>");

                    float mid_x = (x1 + x2) / 2.0f;
                    float mid_y = (y1 + y2) / 2.0f;
                    if (node->wire_length() > 0) {
                        svg_elements.push_back("<text x=\"" + std::to_string(mid_x) + "\" y=\""
                                               + std::to_string(mid_y - 5)
                                               + "\" class=\"wire-label\" text-anchor=\"middle\">"
                                               + std::to_string(node->wire_length()) + "</text>");
                    }
                }
                if (node->left()) draw_wires_recursive(node->left().value());
                if (node->right()) draw_wires_recursive(node->right().value());
            };
            draw_wires_recursive(root);
            return svg_elements;
        }

        /**
         * @brief Draw all nodes in the clock tree.
         */
        auto draw_nodes(const TreeNode* root, const std::vector<Sink>& sinks,
                        const std::function<std::pair<float, float>(float, float)>& scale_coord)
            -> std::vector<std::string> {
            std::vector<std::string> svg_elements;
            std::unordered_set<std::pair<int, int>, PairHash> sink_positions;
            for (const auto& sink : sinks) {
                sink_positions.emplace(sink.position().xcoord(), sink.position().ycoord());
            }

            std::function<void(const TreeNode*, int)> draw_nodes_recursive
                = [&](const TreeNode* node, int depth) {
                      if (!node) return;

                      auto [x, y] = scale_coord(static_cast<float>(node->position().xcoord()),
                                                static_cast<float>(node->position().ycoord()));
                      bool is_sink = sink_positions.count(
                                         {node->position().xcoord(), node->position().ycoord()})
                                     > 0;
                      bool is_root = !node->parent();

                      std::string color;
                      int radius;
                      if (is_root) {
                          color = root_color_;
                          radius = node_radius_ + 2;
                      } else if (is_sink) {
                          color = sink_color_;
                          radius = node_radius_;
                      } else {
                          color = internal_color_;
                          radius = node_radius_ - 2;
                      }

                      svg_elements.push_back("<circle cx=\"" + std::to_string(x) + "\" cy=\""
                                             + std::to_string(y) + "\" r=\""
                                             + std::to_string(radius) + "\" fill=\"" + color
                                             + "\" stroke=\"#333\" stroke-width=\"1\"/>");

                      float label_y_offset = static_cast<float>(-radius - 5);
                      svg_elements.push_back("<text x=\"" + std::to_string(x) + "\" y=\""
                                             + std::to_string(y + label_y_offset)
                                             + "\" class=\"node-label\" text-anchor=\"middle\">"
                                             + node->name() + "</text>");

                      float delay_y_offset = static_cast<float>(radius + 12);
                      svg_elements.push_back("<text x=\"" + std::to_string(x) + "\" y=\""
                                             + std::to_string(y + delay_y_offset)
                                             + "\" class=\"delay-label\" text-anchor=\"middle\">d:"
                                             + std::to_string(node->delay()) + "</text>");

                      if (is_sink) {
                          float cap_y_offset = static_cast<float>(radius + 22);
                          svg_elements.push_back(
                              "<text x=\"" + std::to_string(x) + "\" y=\""
                              + std::to_string(y + cap_y_offset)
                              + "\" class=\"delay-label\" text-anchor=\"middle\">c:"
                              + std::to_string(node->capacitance()) + "</text>");
                      }

                      if (node->left()) draw_nodes_recursive(node->left().value(), depth + 1);
                      if (node->right()) draw_nodes_recursive(node->right().value(), depth + 1);
                  };

            draw_nodes_recursive(root, 0);
            return svg_elements;
        }

        /**
         * @brief Create analysis information box.
         */
        auto create_analysis_box(
            const std::unordered_map<
                std::string, std::variant<float, std::string, std::vector<float>>>& analysis)
            -> std::vector<std::string> {
            std::string delay_model
                = std::holds_alternative<std::string>(analysis.at("delay_model"))
                      ? std::get<std::string>(analysis.at("delay_model"))
                      : "Unknown";

            std::vector<std::string> analysis_text
                = {"Clock Tree Analysis",
                   "Delay Model: " + delay_model,
                   "Max Delay: " + std::to_string(std::get<float>(analysis.at("max_delay"))),
                   "Min Delay: " + std::to_string(std::get<float>(analysis.at("min_delay"))),
                   "Skew: " + std::to_string(std::get<float>(analysis.at("skew"))),
                   "Total Wirelength: "
                       + std::to_string(std::get<float>(analysis.at("total_wirelength"))),
                   "Sinks: "
                       + std::to_string(
                           std::get<std::vector<float>>(analysis.at("sink_delays")).size())};

            std::vector<std::string> analysis_box
                = {"<g class=\"analysis-info\">",
                   "<rect x=\"10\" y=\"10\" width=\"220\" height=\"140\" fill=\"white\" "
                   "stroke=\"#ccc\" stroke-width=\"1\" rx=\"5\"/>",
                   "<rect x=\"10\" y=\"10\" width=\"220\" height=\"20\" fill=\"#f0f0f0\" "
                   "stroke=\"#ccc\" stroke-width=\"1\" rx=\"5\"/>",
                   "<text x=\"20\" y=\"25\" font-family=\"sans-serif\" font-size=\"12\" "
                   "font-weight=\"bold\" fill=\"#333\">Clock Tree Analysis</text>",
                   "<text x=\"20\" y=\"45\" font-family=\"monospace\" font-size=\"11\" "
                   "fill=\"#333\">"};

            for (size_t i = 1; i < analysis_text.size(); ++i) {
                analysis_box.push_back("<tspan x=\"20\" y=\"" + std::to_string(45 + i * 16) + "\">"
                                       + analysis_text[i] + "</tspan>");
            }

            analysis_box.push_back("</text>");
            analysis_box.push_back("</g>");
            return analysis_box;
        }

        int margin_;
        int node_radius_;
        int wire_width_;
        std::string sink_color_;
        std::string internal_color_;
        std::string root_color_;
        std::string wire_color_;
        std::string text_color_;
    };

    /**
     * @brief Create an interactive SVG with additional information and styling.
     */
    inline auto create_interactive_svg(
        const TreeNode* root, const std::vector<Sink>& sinks,
        const std::optional<
            std::unordered_map<std::string, std::variant<float, std::string, std::vector<float>>>>&
            analysis,
        const std::string& filename = "clock_tree_interactive.svg", int width = 1000,
        int height = 700) -> std::string {
        ClockTreeVisualizer visualizer(60, 10, 3, "#2E7D32", "#1565C0", "#C62828", "#455A64",
                                       "#263238");
        return visualizer.visualize_tree(root, sinks, filename, width, height, analysis);
    }

    /**
     * @brief Create a comparison visualization of multiple clock trees.
     */
    inline auto create_comparison_visualization(
        const std::vector<std::unordered_map<
            std::string,
            std::variant<const TreeNode*, std::vector<Sink>,
                         std::unordered_map<std::string,
                                            std::variant<float, std::string, std::vector<float>>>,
                         std::string>>>& trees_data,
        const std::string& filename = "clock_tree_comparison.svg", int width = 1200,
        int height = 800) -> std::string {
        if (trees_data.empty()) {
            throw std::invalid_argument("No tree data provided for comparison");
        }

        size_t num_trees = trees_data.size();
        size_t cols = std::min<size_t>(2, num_trees);
        size_t rows = (num_trees + cols - 1) / cols;

        int sub_width = static_cast<int>(width / cols);
        int sub_height = static_cast<int>(height / rows);

        std::vector<std::string> svg_content
            = {"<svg width=\"" + std::to_string(width) + "\" height=\"" + std::to_string(height)
                   + "\" xmlns=\"http://www.w3.org/2000/svg\">",
               "<style>",
               "  .node-label { font: 8px sans-serif; fill: #333; }",
               "  .delay-label { font: 7px sans-serif; fill: #666; }",
               "  .title { font: 14px sans-serif; fill: #333; font-weight: bold; }",
               "  .comparison-label { font: 10px sans-serif; fill: #333; }",
               "</style>",
               "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>"};

        ClockTreeVisualizer visualizer(40, 6, 2, "#4CAF50", "#2196F3", "#F44336");

        for (size_t i = 0; i < num_trees; ++i) {
            size_t row = i / cols;
            size_t col = i % cols;
            int offset_x = static_cast<int>(col * sub_width);
            int offset_y = static_cast<int>(row * sub_height);

            svg_content.push_back("<text x=\"" + std::to_string(offset_x + sub_width / 2)
                                  + "\" y=\"" + std::to_string(offset_y + 20)
                                  + "\" class=\"title\" text-anchor=\"middle\">"
                                  + std::get<std::string>(trees_data[i].at("title")) + "</text>");

            auto temp_svg = visualizer.visualize_tree(
                std::get<const TreeNode*>(trees_data[i].at("tree")),
                std::get<std::vector<Sink>>(trees_data[i].at("sinks")), "", sub_width - 20,
                sub_height - 40,
                std::get<std::unordered_map<std::string,
                                            std::variant<float, std::string, std::vector<float>>>>(
                    trees_data[i].at("analysis")));

            std::vector<std::string> lines = split_string(temp_svg, '\n');
            int start_idx = -1, end_idx = -1;
            for (size_t j = 0; j < lines.size(); ++j) {
                if (lines[j].find("<g class=\"clock-tree\">") != std::string::npos) {
                    start_idx = static_cast<int>(j);
                } else if (start_idx != -1 && lines[j].find("</g>") != std::string::npos
                           && lines[j - 1].find("clock-tree") == std::string::npos) {
                    end_idx = static_cast<int>(j);
                    break;
                }
            }

            if (start_idx == -1 || end_idx == -1) {
                throw std::runtime_error("Could not parse temporary SVG content");
            }

            std::vector<std::string> tree_content(lines.begin() + start_idx + 1,
                                                  lines.begin() + end_idx);
            if (!tree_content.empty()
                && tree_content[0].find("<rect width=\"100%\" height=\"100%\" fill=\"white\"/>")
                       != std::string::npos) {
                tree_content.erase(tree_content.begin());
            }

            svg_content.push_back("<g transform=\"translate(" + std::to_string(offset_x + 10) + ", "
                                  + std::to_string(offset_y + 40) + ")\">");
            svg_content.insert(svg_content.end(), tree_content.begin(), tree_content.end());
            svg_content.push_back("</g>");
        }

        svg_content.push_back("</svg>");
        std::string svg_string = join_strings(svg_content, "\n");

        std::ofstream file(filename);
        file << svg_string;
        file.close();
        std::cout << "Comparison visualization saved to " << filename << std::endl;

        return svg_string;
    }

    /**
     * @brief Create a specialized comparison between linear and Elmore delay models.
     */
    inline auto create_delay_model_comparison(
        const std::unordered_map<
            std::string,
            std::variant<const TreeNode*, std::vector<Sink>,
                         std::unordered_map<std::string,
                                            std::variant<float, std::string, std::vector<float>>>,
                         std::string>>& linear_tree_data,
        const std::unordered_map<
            std::string,
            std::variant<const TreeNode*, std::vector<Sink>,
                         std::unordered_map<std::string,
                                            std::variant<float, std::string, std::vector<float>>>,
                         std::string>>& elmore_tree_data,
        const std::string& filename = "delay_model_comparison.svg") -> std::string {
        auto linear_data = linear_tree_data;
        auto elmore_data = elmore_tree_data;
        linear_data["title"] = "Linear Delay Model";
        elmore_data["title"] = "Elmore Delay Model";
        return create_comparison_visualization({linear_data, elmore_data}, filename, 1200, 600);
    }

}  // namespace recti
