#pragma once

#include <iomanip>
#include <memory>
#include <vector>
#include <string>
#include <set>
#include <functional>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>

// Include the DME algorithm headers
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
    /**
     * @brief Initialize the visualizer with styling parameters
     */
    ClockTreeVisualizer(
        int margin = 50,
        int node_radius = 8,
        int wire_width = 2,
        const std::string& sink_color = "#4CAF50",
        const std::string& internal_color = "#2196F3",
        const std::string& root_color = "#F44336",
        const std::string& wire_color = "#666666",
        const std::string& text_color = "#333333"
    ) : margin(margin), node_radius(node_radius), wire_width(wire_width),
        sink_color(sink_color), internal_color(internal_color), root_color(root_color),
        wire_color(wire_color), text_color(text_color) {}

    /**
     * @brief Create an SVG visualization of the clock tree
     */
    std::string visualize_tree(
        const std::shared_ptr<TreeNode>& root,
        const std::vector<Sink>& sinks,
        const std::string& filename = "clock_tree.svg",
        int width = 800,
        int height = 600,
        const SkewAnalysis* analysis = nullptr
    ) {
        // Collect all nodes and calculate bounds
        auto all_nodes = collect_all_nodes(root);
        auto [min_x, min_y, max_x, max_y] = calculate_bounds(all_nodes, sinks);

        // Scale coordinates to fit SVG canvas
        double scale_x = (max_x > min_x) ? (width - 2 * margin) / (max_x - min_x) : 1.0;
        double scale_y = (max_y > min_y) ? (height - 2 * margin) / (max_y - min_y) : 1.0;
        double scale = std::min(scale_x, scale_y);  // Maintain aspect ratio

        auto scale_coord = [&](double x, double y) -> std::pair<double, double> {
            double scaled_x = (x - min_x) * scale + margin;
            double scaled_y = (y - min_y) * scale + margin;
            return {scaled_x, scaled_y};
        };

        // Create SVG content
        std::vector<std::string> svg_content;
        svg_content.push_back(
            "<svg width=\"" + std::to_string(width) + 
            "\" height=\"" + std::to_string(height) + 
            "\" xmlns=\"http://www.w3.org/2000/svg\">"
        );
        svg_content.push_back("<style>");
        svg_content.push_back("  .node-label { font: 10px sans-serif; fill: #333; }");
        svg_content.push_back("  .delay-label { font: 8px sans-serif; fill: #666; }");
        svg_content.push_back("  .wire-label { font: 9px sans-serif; fill: #444; }");
        svg_content.push_back("  .analysis-label { font: 12px sans-serif; fill: #333; }");
        svg_content.push_back("</style>");
        svg_content.push_back("<rect width=\"100%\" height=\"100%\" fill=\"white\"/>");
        svg_content.push_back("<g class=\"clock-tree\">");

        // Draw wires first (so they appear behind nodes)
        auto wires = draw_wires(root, scale_coord);
        svg_content.insert(svg_content.end(), wires.begin(), wires.end());

        // Draw nodes
        auto nodes = draw_nodes(root, sinks, scale_coord);
        svg_content.insert(svg_content.end(), nodes.begin(), nodes.end());

        // Add analysis information if provided
        if (analysis) {
            auto analysis_box = create_analysis_box(*analysis, width);
            svg_content.insert(svg_content.end(), analysis_box.begin(), analysis_box.end());
        }

        // Close SVG
        svg_content.push_back("</g>");
        svg_content.push_back("</svg>");

        std::string svg_string;
        for (const auto& line : svg_content) {
            svg_string += line + "\n";
        }

        // Save to file
        if (!filename.empty()) {
            std::ofstream file(filename);
            if (file.is_open()) {
                file << svg_string;
                std::cout << "Clock tree visualization saved to " << filename << std::endl;
            }
        }

        return svg_string;
    }

private:
    /**
     * @brief Collect all nodes in the tree
     */
    std::vector<std::shared_ptr<TreeNode>> collect_all_nodes(const std::shared_ptr<TreeNode>& root) {
        std::vector<std::shared_ptr<TreeNode>> nodes;
        
        std::function<void(const std::shared_ptr<TreeNode>&)> collect;
        collect = [&](const std::shared_ptr<TreeNode>& node) {
            if (node) {
                nodes.push_back(node);
                collect(node->left);
                collect(node->right);
            }
        };
        
        collect(root);
        return nodes;
    }

    /**
     * @brief Calculate the bounding box of all nodes and sinks
     */
    std::tuple<double, double, double, double> calculate_bounds(
        const std::vector<std::shared_ptr<TreeNode>>& nodes,
        const std::vector<Sink>& sinks
    ) {
        if (nodes.empty() && sinks.empty()) {
            return {0.0, 0.0, 100.0, 100.0};
        }

        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();

        // Add tree nodes
        for (const auto& node : nodes) {
            double x = static_cast<double>(node->position.xcoord());
            double y = static_cast<double>(node->position.ycoord());
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }

        // Add original sinks
        for (const auto& sink : sinks) {
            double x = static_cast<double>(sink.position.xcoord());
            double y = static_cast<double>(sink.position.ycoord());
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }

        // Add some padding
        double width = max_x - min_x;
        double height = max_y - min_y;
        double padding = std::max({width * 0.1, height * 0.1, 10.0});

        return {
            min_x - padding,
            min_y - padding,
            max_x + padding,
            max_y + padding
        };
    }

    /**
     * @brief Draw all wires in the clock tree
     */
    std::vector<std::string> draw_wires(
        const std::shared_ptr<TreeNode>& root,
        const std::function<std::pair<double, double>(double, double)>& scale_coord
    ) {
        std::vector<std::string> svg_elements;

        std::function<void(const std::shared_ptr<TreeNode>&)> draw_wires_recursive;
        draw_wires_recursive = [&](const std::shared_ptr<TreeNode>& node) {
            if (!node) return;

            if (node->parent) {
                // Draw wire from parent to current node
                auto [x1, y1] = scale_coord(
                    node->parent->position.xcoord(),
                    node->parent->position.ycoord()
                );
                auto [x2, y2] = scale_coord(
                    node->position.xcoord(),
                    node->position.ycoord()
                );

                std::ostringstream line;
                line << "<line x1=\"" << x1 << "\" y1=\"" << y1 
                     << "\" x2=\"" << x2 << "\" y2=\"" << y2
                     << "\" stroke=\"" << wire_color 
                     << "\" stroke-width=\"" << wire_width
                     << "\" stroke-linecap=\"round\"/>";
                svg_elements.push_back(line.str());

                // Add wire length label
                double mid_x = (x1 + x2) / 2;
                double mid_y = (y1 + y2) / 2;
                if (node->wire_length > 0) {
                    std::ostringstream label;
                    label << "<text x=\"" << mid_x << "\" y=\"" << mid_y - 5
                          << "\" class=\"wire-label\" text-anchor=\"middle\">"
                          << node->wire_length << "</text>";
                    svg_elements.push_back(label.str());
                }
            }

            draw_wires_recursive(node->left);
            draw_wires_recursive(node->right);
        };

        draw_wires_recursive(root);
        return svg_elements;
    }

    /**
     * @brief Draw all nodes in the clock tree
     */
    std::vector<std::string> draw_nodes(
        const std::shared_ptr<TreeNode>& root,
        const std::vector<Sink>& sinks,
        const std::function<std::pair<double, double>(double, double)>& scale_coord
    ) {
        std::vector<std::string> svg_elements;
        
        // Create set of sink positions
        std::set<std::pair<int, int>> sink_positions;
        for (const auto& sink : sinks) {
            sink_positions.insert({
                sink.position.xcoord(),
                sink.position.ycoord()
            });
        }

        std::function<void(const std::shared_ptr<TreeNode>&, int)>
        draw_nodes_recursive = [&](const std::shared_ptr<TreeNode>& node, int depth) {
            if (!node) return;

            auto [x, y] = scale_coord(
                node->position.xcoord(),
                node->position.ycoord()
            );

            // Determine node type and color
            bool is_sink = sink_positions.count({
                node->position.xcoord(),
                node->position.ycoord()
            }) > 0;
            bool is_root = !node->parent;

            std::string color;
            int radius;
            if (is_root) {
                color = root_color;
                radius = node_radius + 2;
            } else if (is_sink) {
                color = sink_color;
                radius = node_radius;
            } else {
                color = internal_color;
                radius = node_radius - 2;
            }

            // Draw node circle
            std::ostringstream circle;
            circle << "<circle cx=\"" << x << "\" cy=\"" << y 
                   << "\" r=\"" << radius << "\" fill=\"" << color
                   << "\" stroke=\"#333\" stroke-width=\"1\"/>";
            svg_elements.push_back(circle.str());

            // Draw node label
            double label_y_offset = -radius - 5;
            std::ostringstream label;
            label << "<text x=\"" << x << "\" y=\"" << y + label_y_offset
                  << "\" class=\"node-label\" text-anchor=\"middle\">"
                  << node->name << "</text>";
            svg_elements.push_back(label.str());

            // Draw delay information
            double delay_y_offset = radius + 12;
            std::ostringstream delay_label;
            delay_label << "<text x=\"" << x << "\" y=\"" << y + delay_y_offset
                       << "\" class=\"delay-label\" text-anchor=\"middle\">d:"
                       << std::fixed << std::setprecision(1) << node->delay << "</text>";
            svg_elements.push_back(delay_label.str());

            // Draw capacitance information for sinks
            if (is_sink) {
                double cap_y_offset = radius + 22;
                std::ostringstream cap_label;
                cap_label << "<text x=\"" << x << "\" y=\"" << y + cap_y_offset
                         << "\" class=\"delay-label\" text-anchor=\"middle\">c:"
                         << std::fixed << std::setprecision(1) << node->capacitance << "</text>";
                svg_elements.push_back(cap_label.str());
            }

            draw_nodes_recursive(node->left, depth + 1);
            draw_nodes_recursive(node->right, depth + 1);
        };

        draw_nodes_recursive(root, 0);
        return svg_elements;
    }

    /**
     * @brief Create analysis information box
     */
    std::vector<std::string> create_analysis_box(const SkewAnalysis& analysis, int ) {
        std::vector<std::string> analysis_box;
        
        analysis_box.push_back("<g class=\"analysis-info\">");
        analysis_box.push_back("<rect x=\"10\" y=\"10\" width=\"220\" height=\"140\" fill=\"white\" stroke=\"#ccc\" stroke-width=\"1\" rx=\"5\"/>");
        analysis_box.push_back("<rect x=\"10\" y=\"10\" width=\"220\" height=\"20\" fill=\"#f0f0f0\" stroke=\"#ccc\" stroke-width=\"1\" rx=\"5\"/>");
        analysis_box.push_back("<text x=\"20\" y=\"25\" font-family=\"sans-serif\" font-size=\"12\" font-weight=\"bold\" fill=\"#333\">Clock Tree Analysis</text>");
        analysis_box.push_back("<text x=\"20\" y=\"45\" font-family=\"monospace\" font-size=\"11\" fill=\"#333\">");

        // Format analysis information
        std::vector<std::string> analysis_text = {
            "Delay Model: " + analysis.delay_model,
            "Max Delay: " + std::to_string(analysis.max_delay),
            "Min Delay: " + std::to_string(analysis.min_delay),
            "Skew: " + std::to_string(analysis.skew),
            "Total Wirelength: " + std::to_string(analysis.total_wirelength),
            "Sinks: " + std::to_string(analysis.sink_delays.size())
        };

        for (size_t i = 0; i < analysis_text.size(); ++i) {
            std::ostringstream tspan;
            tspan << "<tspan x=\"20\" y=\"" << 45 + (i + 1) * 16 << "\">" 
                  << analysis_text[i] << "</tspan>";
            analysis_box.push_back(tspan.str());
        }

        analysis_box.push_back("</text>");
        analysis_box.push_back("</g>");

        return analysis_box;
    }
};

/**
 * @brief Create an interactive SVG with additional information and styling
 */
inline std::string create_interactive_svg(
    const std::shared_ptr<TreeNode>& root,
    const std::vector<Sink>& sinks,
    const SkewAnalysis* analysis = nullptr,
    const std::string& filename = "clock_tree_interactive.svg",
    int width = 1000,
    int height = 700
) {
    ClockTreeVisualizer visualizer(
        60,  // margin
        10,  // node_radius
        3,   // wire_width
        "#2E7D32",  // sink_color
        "#1565C0",  // internal_color
        "#C62828",  // root_color
        "#455A64",  // wire_color
        "#263238"   // text_color
    );

    return visualizer.visualize_tree(root, sinks, filename, width, height, analysis);
}

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
 */
inline std::string create_comparison_visualization(
    const std::vector<TreeComparisonData>& trees_data,
    const std::string& filename = "clock_tree_comparison.svg",
    int width = 1200,
    int height = 800
) {
    if (trees_data.empty()) {
        throw std::invalid_argument("No tree data provided for comparison");
    }

    size_t num_trees = trees_data.size();
    int cols = std::min(2, static_cast<int>(num_trees));  // Maximum 2 columns
    int rows = (static_cast<int>(num_trees) + cols - 1) / cols;

    int sub_width = width / cols;
    int sub_height = height / rows;

    std::vector<std::string> svg_content;
    svg_content.push_back(
        "<svg width=\"" + std::to_string(width) + 
        "\" height=\"" + std::to_string(height) + 
        "\" xmlns=\"http://www.w3.org/2000/svg\">"
    );
    svg_content.push_back("<style>");
    svg_content.push_back("  .node-label { font: 8px sans-serif; fill: #333; }");
    svg_content.push_back("  .delay-label { font: 7px sans-serif; fill: #666; }");
    svg_content.push_back("  .title { font: 14px sans-serif; fill: #333; font-weight: bold; }");
    svg_content.push_back("  .comparison-label { font: 10px sans-serif; fill: #333; }");
    svg_content.push_back("</style>");
    svg_content.push_back("<rect width=\"100%\" height=\"100%\" fill=\"white\"/>");

    ClockTreeVisualizer visualizer(
        40,  // margin
        6,   // node_radius
        2,   // wire_width
        "#4CAF50",  // sink_color
        "#2196F3",  // internal_color
        "#F44336"   // root_color
    );

    for (size_t i = 0; i < trees_data.size(); ++i) {
        int row = static_cast<int>(i) / cols;
        int col = static_cast<int>(i) % cols;

        int offset_x = col * sub_width;
        int offset_y = row * sub_height;

        // Add title for this subplot
        std::ostringstream title;
        title << "<text x=\"" << offset_x + sub_width / 2 
              << "\" y=\"" << offset_y + 20 
              << "\" class=\"title\" text-anchor=\"middle\">"
              << trees_data[i].title << "</text>";
        svg_content.push_back(title.str());

        // Create visualization for this tree
        auto temp_svg = visualizer.visualize_tree(
            trees_data[i].tree,
            trees_data[i].sinks,
            "",  // No filename for temporary use
            sub_width - 20,
            sub_height - 40,
            &trees_data[i].analysis
        );

        // Extract the main content (between <g class="clock-tree"> and </g>)
        std::istringstream stream(temp_svg);
        std::string line;
        std::vector<std::string> tree_lines;
        bool in_clock_tree = false;
        
        while (std::getline(stream, line)) {
            if (line.find("<g class=\"clock-tree\">") != std::string::npos) {
                in_clock_tree = true;
                continue;
            }
            if (in_clock_tree && line.find("</g>") != std::string::npos) {
                break;
            }
            if (in_clock_tree) {
                tree_lines.push_back(line);
            }
        }

        // Remove the background rect if present
        if (!tree_lines.empty() && 
            tree_lines[0].find("<rect width=\"100%\" height=\"100%\" fill=\"white\"/>") != std::string::npos) {
            tree_lines.erase(tree_lines.begin());
        }

        // Add transformed group
        std::ostringstream transform;
        transform << "<g transform=\"translate(" << offset_x + 10 
                  << ", " << offset_y + 40 << ")\">";
        svg_content.push_back(transform.str());
        svg_content.insert(svg_content.end(), tree_lines.begin(), tree_lines.end());
        svg_content.push_back("</g>");
        svg_content.push_back("</g>"); // luk
    }

    svg_content.push_back("</svg>");

    std::string svg_string;
    for (const auto& line : svg_content) {
        svg_string += line + "\n";
    }

    if (!filename.empty()) {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << svg_string;
            std::cout << "Comparison visualization saved to " << filename << std::endl;
        }
    }

    return svg_string;
}

/**
 * @brief Create a specialized comparison between linear and Elmore delay models
 */
inline std::string create_delay_model_comparison(
    const TreeComparisonData& linear_tree_data,
    const TreeComparisonData& elmore_tree_data,
    const std::string& filename = "delay_model_comparison.svg"
) {
    TreeComparisonData linear_data = linear_tree_data;
    TreeComparisonData elmore_data = elmore_tree_data;
    
    linear_data.title = "Linear Delay Model";
    elmore_data.title = "Elmore Delay Model";

    return create_comparison_visualization(
        {linear_data, elmore_data}, filename, 1200, 600
    );
}

/**
 * @brief Example function demonstrating clock tree visualization with different delay models
 */
inline void visualize_example_tree() {
    // Create example sinks
    std::vector<Sink> example_sinks = {
        Sink("s1", Point<int>(-100, 40), 1.0),
        Sink("s2", Point<int>(-60, 60), 1.0),
        Sink("s3", Point<int>(0, 40), 1.0),
        Sink("s4", Point<int>(20, 20), 1.0),
        Sink("s5", Point<int>(-20, -20), 1.0),
        Sink("s6", Point<int>(-30, -50), 1.0),
        Sink("s7", Point<int>(-100, -40), 1.0),
        Sink("s8", Point<int>(-100, 0), 1.0)
    };

    std::cout << "=== Generating Clock Trees with Different Delay Models ===" << std::endl;

    // Linear delay model
    auto linear_calc = std::make_unique<LinearDelayCalculator>(0.5, 0.2);
    DMEAlgorithm dme_linear(example_sinks, std::move(linear_calc));
    auto clock_tree_linear = dme_linear.build_clock_tree();
    auto analysis_linear = dme_linear.analyze_skew(clock_tree_linear);

    // Elmore delay model
    auto elmore_calc = std::make_unique<ElmoreDelayCalculator>(0.1, 0.2);
    DMEAlgorithm dme_elmore(example_sinks, std::move(elmore_calc));
    auto clock_tree_elmore = dme_elmore.build_clock_tree();
    auto analysis_elmore = dme_elmore.analyze_skew(clock_tree_elmore);

    // Create individual visualizations
    ClockTreeVisualizer visualizer;

    // Linear model visualization
    auto linear_svg = visualizer.visualize_tree(
        clock_tree_linear,
        example_sinks,
        "linear_model_clock_tree.svg",
        800, 600,
        &analysis_linear
    );

    // Elmore model visualization
    auto elmore_svg = visualizer.visualize_tree(
        clock_tree_elmore,
        example_sinks,
        "elmore_model_clock_tree.svg",
        800, 600,
        &analysis_elmore
    );

    // Comparison visualization
    TreeComparisonData linear_data = {
        clock_tree_linear,
        example_sinks,
        analysis_linear,
        "Linear Delay Model"
    };
    
    TreeComparisonData elmore_data = {
        clock_tree_elmore,
        example_sinks,
        analysis_elmore,
        "Elmore Delay Model"
    };

    auto comparison_svg = create_delay_model_comparison(linear_data, elmore_data);

    std::cout << "Visualizations created:" << std::endl;
    std::cout << "- linear_model_clock_tree.svg: Linear delay model" << std::endl;
    std::cout << "- elmore_model_clock_tree.svg: Elmore delay model" << std::endl;
    std::cout << "- delay_model_comparison.svg: Side-by-side comparison" << std::endl;
}

} // namespace recti