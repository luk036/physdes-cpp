#include "recti/visualize_clock_tree.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace recti {

    // -----------------------------------------------------------------------
    // ClockTreeVisualizer
    // -----------------------------------------------------------------------
    std::string ClockTreeVisualizer::visualize_tree(const Tree& tree, NodeIdx root,
                                                    const std::vector<Sink>& sinks,
                                                    const std::string& filename, int width,
                                                    int height, const SkewAnalysis* analysis) {
        auto all_nodes = collect_all_nodes(tree, root);
        auto [min_x, min_y, max_x, max_y] = calculate_bounds(all_nodes, sinks, tree);

        double scale_x = (max_x > min_x) ? (width - 2 * margin) / (max_x - min_x) : 1.0;
        double scale_y = (max_y > min_y) ? (height - 2 * margin) / (max_y - min_y) : 1.0;
        double scale = std::min(scale_x, scale_y);

        auto scale_coord = [&](double coord_x, double coord_y) -> std::pair<double, double> {
            double scaled_x = (coord_x - min_x) * scale + margin;
            double scaled_y = (coord_y - min_y) * scale + margin;
            return {scaled_x, scaled_y};
        };

        std::vector<std::string> svg_content;
        svg_content.emplace_back("<svg width=\"" + std::to_string(width) + "\" height=\""
                                 + std::to_string(height)
                                 + R"(" xmlns="http://www.w3.org/2000/svg">)");
        svg_content.emplace_back("<style>");
        svg_content.emplace_back("  .node-label { font: 10px sans-serif; fill: #333; }");
        svg_content.emplace_back("  .delay-label { font: 8px sans-serif; fill: #666; }");
        svg_content.emplace_back("  .wire-label { font: 9px sans-serif; fill: #444; }");
        svg_content.emplace_back("  .analysis-label { font: 12px sans-serif; fill: #333; }");
        svg_content.emplace_back("</style>");
        svg_content.emplace_back(R"(<rect width="100%" height="100%" fill="white"/>)");
        svg_content.emplace_back("<g class=\"clock-tree\">");

        auto wires = draw_wires(root, tree, scale_coord);
        svg_content.insert(svg_content.end(), wires.begin(), wires.end());

        auto nodes = draw_nodes(root, tree, sinks, scale_coord);
        svg_content.insert(svg_content.end(), nodes.begin(), nodes.end());

        if (analysis != nullptr) {
            auto analysis_box = create_analysis_box(*analysis, width);
            svg_content.insert(svg_content.end(), analysis_box.begin(), analysis_box.end());
        }

        svg_content.emplace_back("</g>");
        svg_content.emplace_back("</svg>");

        std::string svg_string;
        for (const auto& line : svg_content) {
            svg_string += line + "\n";
        }

        if (!filename.empty()) {
            std::ofstream file(filename);
            if (file.is_open()) {
                file << svg_string;
                std::cout << "Clock tree visualization saved to " << filename << '\n';
            }
        }

        return svg_string;
    }

    std::vector<NodeIdx> ClockTreeVisualizer::collect_all_nodes(const Tree& tree, NodeIdx root) {
        std::vector<NodeIdx> indices;
        std::function<void(NodeIdx)> collect;
        collect = [&](NodeIdx n) {
            if (n == SIZE_MAX) return;
            indices.push_back(n);
            collect(tree.get(n).left);
            collect(tree.get(n).right);
        };
        collect(root);
        return indices;
    }

    std::tuple<double, double, double, double> ClockTreeVisualizer::calculate_bounds(
        const std::vector<NodeIdx>& node_indices, const std::vector<Sink>& sinks,
        const Tree& tree) {
        if (node_indices.empty() && sinks.empty()) {
            return {0.0, 0.0, 100.0, 100.0};
        }

        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();

        for (NodeIdx idx : node_indices) {
            auto x = static_cast<double>(tree.get(idx).position.xcoord());
            auto y = static_cast<double>(tree.get(idx).position.ycoord());
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }

        for (const auto& sink : sinks) {
            auto x = static_cast<double>(sink.position.xcoord());
            auto y = static_cast<double>(sink.position.ycoord());
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }

        double width = max_x - min_x;
        double height = max_y - min_y;
        double padding = std::max({width * 0.1, height * 0.1, 10.0});

        return {min_x - padding, min_y - padding, max_x + padding, max_y + padding};
    }

    std::vector<std::string> ClockTreeVisualizer::draw_wires(
        NodeIdx root, const Tree& tree,
        const std::function<std::pair<double, double>(double, double)>& scale_coord) {
        std::vector<std::string> svg_elements;

        std::function<void(NodeIdx)> draw_recursive;
        draw_recursive = [&](NodeIdx n) {
            if (n == SIZE_MAX) return;
            const auto& node = tree.get(n);

            if (node.parent != SIZE_MAX) {
                const auto& parent = tree.get(node.parent);
                auto [x1, y1]
                    = scale_coord(parent.position.xcoord(), parent.position.ycoord());
                auto [x2, y2]
                    = scale_coord(node.position.xcoord(), node.position.ycoord());

                std::ostringstream line;
                line << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2
                     << "\" y2=\"" << y2 << "\" stroke=\"" << this->wire_color
                     << "\" stroke-width=\"" << this->wire_width
                     << R"(" stroke-linecap="round"/>)";
                svg_elements.emplace_back(line.str());

                if (node.wire_length > 0) {
                    double mx = (x1 + x2) / 2;
                    double my = (y1 + y2) / 2;
                    std::ostringstream label;
                    label << "<text x=\"" << mx << "\" y=\"" << my - 5
                          << R"(" class="wire-label" text-anchor="middle">)"
                          << node.wire_length << "</text>";
                    svg_elements.emplace_back(label.str());
                }
            }

            draw_recursive(node.left);
            draw_recursive(node.right);
        };

        draw_recursive(root);
        return svg_elements;
    }

    std::vector<std::string> ClockTreeVisualizer::draw_nodes(
        NodeIdx root, const Tree& tree, const std::vector<Sink>& sinks,
        const std::function<std::pair<double, double>(double, double)>& scale_coord) {
        std::vector<std::string> svg_elements;

        std::set<std::pair<int, int>> sink_positions;
        for (const auto& sink : sinks) {
            sink_positions.insert({sink.position.xcoord(), sink.position.ycoord()});
        }

        std::function<void(NodeIdx)> draw_recursive;
        draw_recursive = [&](NodeIdx n) {
            if (n == SIZE_MAX) return;
            const auto& node = tree.get(n);

            auto [cx, cy]
                = scale_coord(node.position.xcoord(), node.position.ycoord());

            bool is_sink = sink_positions.contains(
                {node.position.xcoord(), node.position.ycoord()});
            bool is_root = (node.parent == SIZE_MAX);

            std::string color;
            int r = 0;
            if (is_root) {
                color = this->root_color;
                r = this->node_radius + 2;
            } else if (is_sink) {
                color = this->sink_color;
                r = this->node_radius;
            } else {
                color = this->internal_color;
                r = this->node_radius - 2;
            }

            std::ostringstream circle;
            circle << "<circle cx=\"" << cx << "\" cy=\"" << cy << "\" r=\"" << r
                   << "\" fill=\"" << color << R"(" stroke="#333" stroke-width="1"/>)";
            svg_elements.emplace_back(circle.str());

            double label_y_off = -r - 5;
            std::ostringstream label;
            label << "<text x=\"" << cx << "\" y=\"" << cy + label_y_off
                  << R"(" class="node-label" text-anchor="middle">)" << node.name
                  << "</text>";
            svg_elements.emplace_back(label.str());

            double delay_y_off = r + 12;
            std::ostringstream delay_label;
            delay_label << "<text x=\"" << cx << "\" y=\"" << cy + delay_y_off
                        << R"(" class="delay-label" text-anchor="middle">d:)"
                        << std::fixed << std::setprecision(1) << node.delay << "</text>";
            svg_elements.emplace_back(delay_label.str());

            if (is_sink) {
                double cap_y_off = r + 22;
                std::ostringstream cap_label;
                cap_label << "<text x=\"" << cx << "\" y=\"" << cy + cap_y_off
                          << R"(" class="delay-label" text-anchor="middle">c:)"
                          << std::fixed << std::setprecision(1) << node.capacitance
                          << "</text>";
                svg_elements.emplace_back(cap_label.str());
            }

            draw_recursive(node.left);
            draw_recursive(node.right);
        };

        draw_recursive(root);
        return svg_elements;
    }

    std::vector<std::string> ClockTreeVisualizer::create_analysis_box(
        const SkewAnalysis& analysis, int /* width */) {
        std::vector<std::string> box;
        box.emplace_back("<g class=\"analysis-info\">");
        box.emplace_back(
            "<rect x=\"10\" y=\"10\" width=\"220\" height=\"140\" fill=\"white\" "
            "stroke=\"#ccc\" stroke-width=\"1\" rx=\"5\"/>");
        box.emplace_back(
            "<rect x=\"10\" y=\"10\" width=\"220\" height=\"20\" fill=\"#f0f0f0\" "
            "stroke=\"#ccc\" stroke-width=\"1\" rx=\"5\"/>");
        box.emplace_back(
            "<text x=\"20\" y=\"25\" font-family=\"sans-serif\" font-size=\"12\" "
            "font-weight=\"bold\" fill=\"#333\">Clock Tree Analysis</text>");
        box.emplace_back(
            "<text x=\"20\" y=\"45\" font-family=\"monospace\" font-size=\"11\" "
            "fill=\"#333\">");

        std::vector<std::string> lines
            = {"Delay Model: " + analysis.delay_model,
               "Max Delay: " + std::to_string(analysis.max_delay),
               "Min Delay: " + std::to_string(analysis.min_delay),
               "Skew: " + std::to_string(analysis.skew),
               "Total Wirelength: " + std::to_string(analysis.total_wirelength),
               "Sinks: " + std::to_string(analysis.sink_delays.size())};

        for (size_t i = 0; i < lines.size(); ++i) {
            std::ostringstream tspan;
            tspan << R"(<tspan x="20" y=")" << 45 + (static_cast<int>(i) + 1) * 16 << "\">"
                  << lines[i] << "</tspan>";
            box.emplace_back(tspan.str());
        }

        box.emplace_back("</text>");
        box.emplace_back("</g>");
        return box;
    }

    // -----------------------------------------------------------------------
    // Free functions
    // -----------------------------------------------------------------------
    std::string create_interactive_svg(const Tree& tree, NodeIdx root,
                                       const std::vector<Sink>& sinks,
                                       const SkewAnalysis* analysis,
                                       const std::string& filename, int width, int height) {
        ClockTreeVisualizer viz(60, 10, 3, "#2E7D32", "#1565C0", "#C62828", "#455A64",
                                "#263238");
        return viz.visualize_tree(tree, root, sinks, filename, width, height, analysis);
    }

    std::string create_comparison_visualization(const std::vector<TreeComparisonData>& trees_data,
                                                const std::string& filename, int width,
                                                int height) {
        if (trees_data.empty()) {
            throw std::invalid_argument("No tree data provided for comparison");
        }

        size_t num_trees = trees_data.size();
        int cols = std::min(2, static_cast<int>(num_trees));
        int rows = (static_cast<int>(num_trees) + cols - 1) / cols;
        int sub_w = width / cols;
        int sub_h = height / rows;

        std::vector<std::string> svg;
        svg.emplace_back("<svg width=\"" + std::to_string(width) + "\" height=\""
                         + std::to_string(height)
                         + R"(" xmlns="http://www.w3.org/2000/svg">)");
        svg.emplace_back("<style>");
        svg.emplace_back("  .node-label { font: 8px sans-serif; fill: #333; }");
        svg.emplace_back("  .delay-label { font: 7px sans-serif; fill: #666; }");
        svg.emplace_back("  .title { font: 14px sans-serif; fill: #333; font-weight: bold; }");
        svg.emplace_back("  .comparison-label { font: 10px sans-serif; fill: #333; }");
        svg.emplace_back("</style>");
        svg.emplace_back(R"(<rect width="100%" height="100%" fill="white"/>)");

        ClockTreeVisualizer viz(40, 6, 2, "#4CAF50", "#2196F3", "#F44336");

        for (size_t i = 0; i < trees_data.size(); ++i) {
            int row = static_cast<int>(i) / cols;
            int col = static_cast<int>(i) % cols;
            int ox = col * sub_w;
            int oy = row * sub_h;

            std::ostringstream title;
            title << "<text x=\"" << ox + sub_w / 2 << "\" y=\"" << oy + 20
                  << R"(" class="title" text-anchor="middle">)" << trees_data[i].title
                  << "</text>";
            svg.emplace_back(title.str());

            auto temp_svg = viz.visualize_tree(*trees_data[i].tree, trees_data[i].root,
                                               trees_data[i].sinks, "", sub_w - 20,
                                               sub_h - 40, &trees_data[i].analysis);

            std::istringstream stream(temp_svg);
            std::string line;
            std::vector<std::string> tree_lines;
            int g_depth = 0;
            bool in_clock_tree = false;
            while (std::getline(stream, line)) {
                if (!in_clock_tree) {
                    if (line.find("<g class=\"clock-tree\">") != std::string::npos) {
                        in_clock_tree = true;
                    }
                    continue;
                }
                if (line.find("<g ") != std::string::npos
                    && line.find("</g>") == std::string::npos) {
                    ++g_depth;
                }
                if (line.find("</g>") != std::string::npos) {
                    if (g_depth == 0) break;
                    --g_depth;
                }
                tree_lines.emplace_back(line);
            }

            if (!tree_lines.empty()
                && tree_lines[0].find(R"(<rect width="100%" height="100%" fill="white"/>)")
                       != std::string::npos) {
                tree_lines.erase(tree_lines.begin());
            }

            std::ostringstream transform;
            transform << "<g transform=\"translate(" << ox + 10 << ", " << oy + 40 << ")\">";
            svg.emplace_back(transform.str());
            svg.insert(svg.end(), tree_lines.begin(), tree_lines.end());
            svg.emplace_back("</g>");
        }

        svg.emplace_back("</svg>");

        std::string result;
        for (const auto& line : svg) {
            result += line + "\n";
        }

        if (!filename.empty()) {
            std::ofstream file(filename);
            if (file.is_open()) {
                file << result;
                std::cout << "Comparison visualization saved to " << filename << '\n';
            }
        }

        return result;
    }

    std::string create_delay_model_comparison(const TreeComparisonData& linear_tree_data,
                                              const TreeComparisonData& elmore_tree_data,
                                              const std::string& filename) {
        TreeComparisonData linear_data = linear_tree_data;
        TreeComparisonData elmore_data = elmore_tree_data;
        linear_data.title = "Linear Delay Model";
        elmore_data.title = "Elmore Delay Model";
        return create_comparison_visualization({linear_data, elmore_data}, filename, 1200, 600);
    }

    void visualize_example_tree() {
        std::vector<Sink> sinks
            = {Sink("s1", Point<int>(-100, 40), 1.0),  Sink("s2", Point<int>(-60, 60), 1.0),
               Sink("s3", Point<int>(0, 40), 1.0),     Sink("s4", Point<int>(20, 20), 1.0),
               Sink("s5", Point<int>(-20, -20), 1.0),  Sink("s6", Point<int>(-30, -50), 1.0),
               Sink("s7", Point<int>(-100, -40), 1.0), Sink("s8", Point<int>(-100, 0), 1.0)};

        std::cout << "=== Generating Clock Trees with Different Delay Models ===\n";

        auto linear_calc = std::make_unique<LinearDelayCalculator>(0.5, 0.2);
        DMEAlgorithm dme_linear(sinks, std::move(linear_calc));
        NodeIdx root_linear = dme_linear.build_clock_tree();
        auto analysis_linear = dme_linear.analyze_skew(root_linear);

        auto elmore_calc = std::make_unique<ElmoreDelayCalculator>(0.1, 0.2);
        DMEAlgorithm dme_elmore(sinks, std::move(elmore_calc));
        NodeIdx root_elmore = dme_elmore.build_clock_tree();
        auto analysis_elmore = dme_elmore.analyze_skew(root_elmore);

        ClockTreeVisualizer visualizer;

        auto linear_svg = visualizer.visualize_tree(dme_linear.get_tree(), root_linear,
                                                    sinks, "linear_model_clock_tree.svg",
                                                    800, 600, &analysis_linear);

        auto elmore_svg = visualizer.visualize_tree(dme_elmore.get_tree(), root_elmore,
                                                    sinks, "elmore_model_clock_tree.svg",
                                                    800, 600, &analysis_elmore);

        TreeComparisonData linear_data = {.tree = &dme_linear.get_tree(),
                                          .root = root_linear,
                                          .sinks = sinks,
                                          .analysis = analysis_linear,
                                          .title = "Linear Delay Model"};

        TreeComparisonData elmore_data = {.tree = &dme_elmore.get_tree(),
                                          .root = root_elmore,
                                          .sinks = sinks,
                                          .analysis = analysis_elmore,
                                          .title = "Elmore Delay Model"};

        create_delay_model_comparison(linear_data, elmore_data);

        std::cout << "Visualizations created:\n";
        std::cout << "- linear_model_clock_tree.svg: Linear delay model\n";
        std::cout << "- elmore_model_clock_tree.svg: Elmore delay model\n";
        std::cout << "- delay_model_comparison.svg: Side-by-side comparison\n";
    }

}  // namespace recti
