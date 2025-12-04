// include/recti/detail/svg_utils.hpp
#pragma once

#include <recti/global_router.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace recti::detail {

    /**
     * @brief SVG rendering parameters
     *
     * This structure contains all the parameters needed for generating
     * SVG visualizations of routing trees and geometric objects.
     */
    struct SvgParams {
        int width;      ///< Width of the SVG canvas
        int height;     ///< Height of the SVG canvas
        int margin;     ///< Margin around the drawing area
        double scale;   ///< Scaling factor for coordinates
        int min_x;      ///< Minimum x-coordinate in the data
        int min_y;      ///< Minimum y-coordinate in the data
    };

    /**
     * @brief Calculate SVG rendering parameters
     *
     * Computes the scaling and translation parameters needed to fit the given
     * routing nodes into an SVG canvas of specified dimensions.
     *
     * @tparam IntPoint The point type used in routing nodes
     * @param[in] nodes Vector of routing node pointers to calculate bounds for
     * @param[in] width Desired width of the SVG canvas
     * @param[in] height Desired height of the SVG canvas
     * @param[in] margin Margin to leave around the drawing area
     * @return SvgParams Calculated rendering parameters
     */
    template <typename IntPoint>
    SvgParams calculate_svg_params(const std::vector<RoutingNode<IntPoint>*>& nodes, int width,
                                   int height, int margin);

    /**
     * @brief Specialization for Point<int, int> type
     *
     * This specialization calculates SVG parameters for routing nodes using
     * integer point coordinates. It determines the bounding box of all nodes
     * and computes appropriate scaling to fit within the canvas.
     *
     * @param[in] nodes Vector of routing node pointers with integer coordinates
     * @param[in] width Desired width of the SVG canvas
     * @param[in] height Desired height of the SVG canvas
     * @param[in] margin Margin to leave around the drawing area
     * @return SvgParams Calculated rendering parameters
     */
    template <> inline SvgParams calculate_svg_params<Point<int, int>>(
        const std::vector<RoutingNode<Point<int, int>>*>& nodes, int width, int height,
        int margin) {
        if (nodes.empty()) {
            return {width, height, margin, 1.0, 0, 0};
        }

        int min_x = nodes[0]->pt.xcoord(), max_x = min_x;
        int min_y = nodes[0]->pt.ycoord(), max_y = min_y;
        for (auto node : nodes) {
            min_x = std::min(min_x, node->pt.xcoord());
            max_x = std::max(max_x, node->pt.xcoord());
            min_y = std::min(min_y, node->pt.ycoord());
            max_y = std::max(max_y, node->pt.ycoord());
        }

        int range_x = max_x - min_x;
        int range_y = max_y - min_y;
        if (range_x == 0) range_x = 1;
        if (range_y == 0) range_y = 1;

        double scale_x = (width - 2.0 * margin) / range_x;
        double scale_y = (height - 2.0 * margin) / range_y;
        double scale = std::min(scale_x, scale_y);

        return {width, height, margin, scale, min_x, min_y};
    }

    template <> inline SvgParams calculate_svg_params<Point<Point<int, int>, int>>(
        const std::vector<RoutingNode<Point<Point<int, int>, int>>*>& nodes, int width, int height,
        int margin) {
        if (nodes.empty()) {
            return {width, height, margin, 1.0, 0, 0};
        }

        int min_x = nodes[0]->pt.xcoord().xcoord(), max_x = min_x;
        int min_y = nodes[0]->pt.ycoord(), max_y = min_y;
        for (auto node : nodes) {
            min_x = std::min(min_x, node->pt.xcoord().xcoord());
            max_x = std::max(max_x, node->pt.xcoord().xcoord());
            min_y = std::min(min_y, node->pt.ycoord());
            max_y = std::max(max_y, node->pt.ycoord());
        }

        int range_x = max_x - min_x;
        int range_y = max_y - min_y;
        if (range_x == 0) range_x = 1;
        if (range_y == 0) range_y = 1;

        double scale_x = (width - 2.0 * margin) / range_x;
        double scale_y = (height - 2.0 * margin) / range_y;
        double scale = std::min(scale_x, scale_y);

        return {width, height, margin, scale, min_x, min_y};
    }

    inline std::pair<double, double> scale_coords(int x, int y, const SvgParams& params) {
        double sx = params.margin + (x - params.min_x) * params.scale;
        double sy = params.margin + (y - params.min_y) * params.scale;
        return {sx, sy};
    }

    template <typename IntPoint> void draw_node(std::ostringstream& svg,
                                                const RoutingNode<IntPoint>* node,
                                                const SvgParams& params);

    template <> inline void draw_node<Point<int, int>>(std::ostringstream& svg,
                                                       const RoutingNode<Point<int, int>>* node,
                                                       const SvgParams& params) {
        auto [x, y] = scale_coords(node->pt.xcoord(), node->pt.ycoord(), params);
        std::string color;
        int radius;
        std::string label;
        if (node->type == NodeType::SOURCE) {
            color = std::string("red");
            radius = 8;
            label = std::string("S");
        } else if (node->type == NodeType::STEINER) {
            color = std::string("blue");
            radius = 6;
            size_t pos = node->id.find('_');
            label = std::string("S");
            label += pos != std::string::npos ? node->id.substr(pos + 1) : std::string("");
        } else if (node->type == NodeType::TERMINAL) {
            color = std::string("green");
            radius = 6;
            size_t pos = node->id.find('_');
            label = std::string("T");
            label += pos != std::string::npos ? node->id.substr(pos + 1) : std::string("");
        } else {
            color = std::string("gray");
            radius = 5;
            label = node->id;
        }
        svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << radius << "\" fill=\""
            << color << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
        svg << "<text x=\"" << x + radius + 2 << "\" y=\"" << y + 4
            << "\" font-family=\"Arial\" font-size=\"10\" fill=\"black\">" << label << "</text>\n";
        svg << "<text x=\"" << x << "\" y=\"" << y - radius - 5
            << "\" font-family=\"Arial\" font-size=\"8\" fill=\"gray\" text-anchor=\"middle\">("
            << node->pt << ")</text>\n";
    }

    template <> inline void draw_node<Point<Point<int, int>, int>>(
        std::ostringstream& svg, const RoutingNode<Point<Point<int, int>, int>>* node,
        const SvgParams& params) {
        auto [x, y] = scale_coords(node->pt.xcoord().xcoord(), node->pt.ycoord(), params);
        std::string color;
        int radius;
        std::string label;
        if (node->type == NodeType::SOURCE) {
            color = "red";
            radius = 8;
            label = std::string("S");
        } else if (node->type == NodeType::STEINER) {
            color = "blue";
            radius = 6;
            size_t pos = node->id.find('_');
            label = std::string("S");
            label += pos != std::string::npos ? std::string(node->id.substr(pos + 1))
                                              : std::string("");
        } else if (node->type == NodeType::TERMINAL) {
            color = "green";
            radius = 6;
            size_t pos = node->id.find('_');
            label = std::string("T");
            label += pos != std::string::npos ? std::string(node->id.substr(pos + 1))
                                              : std::string("");
        } else {
            color = "gray";
            radius = 5;
            label = node->id;
        }
        svg << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << radius << "\" fill=\""
            << color << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
        svg << "<text x=\"" << x + radius + 2 << "\" y=\"" << y + 4
            << "\" font-family=\"Arial\" font-size=\"10\" fill=\"black\">" << label << "</text>\n";
        svg << "<text x=\"" << x << "\" y=\"" << y - radius - 5
            << "\" font-family=\"Arial\" font-size=\"8\" fill=\"gray\" text-anchor=\"middle\">("
            << node->pt << ")</text>\n";
    }

    inline void draw_legend(std::ostringstream& svg) {
        int legend_y = 20;
        svg << "<text x=\"20\" y=\"" << legend_y
            << "\" font-family=\"Arial\" font-size=\"12\" font-weight=\"bold\">Legend:</text>\n";

        struct LegendItem {
            std::string text;
            std::string color;
            int x, y;
        };
        std::vector<LegendItem> legend_items = {
            {"Source", "red", 20, legend_y + 20},
            {"Steiner", "blue", 20, legend_y + 40},
            {"Terminal", "green", 20, legend_y + 60},
        };
        for (const auto& item : legend_items) {
            svg << "<circle cx=\"" << item.x << "\" cy=\"" << item.y - 4 << "\" r=\"4\" fill=\""
                << item.color << "\" stroke=\"black\"/>\n";
            svg << "<text x=\"" << item.x + 10 << "\" y=\"" << item.y
                << "\" font-family=\"Arial\" font-size=\"10\">" << item.text << "</text>\n";
        }
    }

    template <typename IntPoint>
    void draw_stats(std::ostringstream& svg, const GlobalRoutingTree<IntPoint>& tree) {
        int stats_y = 110;
        svg << "<text x=\"20\" y=\"" << stats_y
            << "\" font-family=\"Arial\" font-size=\"10\" "
               "font-weight=\"bold\">Statistics:</text>\n";
        svg << "<text x=\"20\" y=\"" << stats_y + 15
            << "\" font-family=\"Arial\" font-size=\"9\">Total Nodes: " << tree.nodes.size()
            << "</text>\n";
        svg << "<text x=\"20\" y=\"" << stats_y + 30
            << "\" font-family=\"Arial\" font-size=\"9\">Terminals: "
            << tree.get_all_terminals().size() << "</text>\n";
        svg << "<text x=\"20\" y=\"" << stats_y + 45
            << "\" font-family=\"Arial\" font-size=\"9\">Steiner: "
            << tree.get_all_steiner_nodes().size() << "</text>\n";
        svg << "<text x=\"20\" y=\"" << stats_y + 60
            << "\" font-family=\"Arial\" font-size=\"9\">Wirelength: "
            << tree.calculate_wirelength() << "</text>\n";
    }

}  // namespace recti::detail
