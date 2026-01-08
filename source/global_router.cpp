// global_router.cpp - Implementation of global routing algorithms
#include <algorithm>
#include <fstream>
#include <iostream>
#include <recti/detail/svg_utils.hpp>
#include <recti/generic.hpp>
#include <recti/global_router.hpp>
#include <recti/halton_int.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <sstream>
#include <string>
#include <vector>

using namespace recti;

/**
 * @brief Stream insertion operator for RoutingNode
 *
 * Outputs a routing node to a stream in a readable format showing
 * the node type, ID, and coordinates.
 *
 * @tparam IntPoint The point type used in the routing node
 * @param[in,out] output_stream The output stream
 * @param[in] routing_node The routing node to output
 * @return std::ostream& Reference to the output stream
 */
template <typename IntPoint>
std::ostream& operator<<(std::ostream& output_stream, const RoutingNode<IntPoint>& routing_node) {
    std::string type_name = to_string(routing_node.type);
    output_stream << type_name << "Node(" << routing_node.id << ", (" << routing_node.pt << "))";
    return output_stream;
}

/**
 * @brief Get tree structure as formatted string
 *
 * Recursively builds a string representation of the routing tree
 * with indentation to show the hierarchical structure.
 *
 * @tparam IntPoint The point type used in routing nodes
 * @param[in] current_node Starting node (nullptr for root)
 * @param[in] level Current indentation level
 * @return std::string Formatted tree structure
 */
template <typename IntPoint>
auto GlobalRoutingTree<IntPoint>::get_tree_structure(const RoutingNode<IntPoint>* current_node,
                                                     int level) const -> std::string {
    if (current_node == nullptr) {
        current_node = &this->source_node;
    }
    std::ostringstream oss;
    oss << std::string(static_cast<size_t>(level) * 2, ' ') << *current_node << "\n";
    for (auto child : current_node->children) {
        oss << this->get_tree_structure(child, level + 1);
    }
    return oss.str();
}

/**
 * @brief Visualize the routing tree structure
 *
 * Outputs a detailed visualization of the routing tree to stdout,
 * including the tree structure, wirelength, node count, and statistics
 * about terminals and Steiner points.
 *
 * @tparam IntPoint The point type used in routing nodes
 */
template <typename IntPoint> void GlobalRoutingTree<IntPoint>::visualize_tree() const {
    std::cout << "Global Routing Tree Structure:\n";
    std::cout << "========================================\n";
    std::cout << this->get_tree_structure();
    std::cout << "Total wirelength: " << this->calculate_wirelength() << "\n";
    std::cout << "Total nodes: " << this->nodes.size() << "\n";
    std::cout << "Terminals: " << this->get_all_terminals().size() << "\n";
    std::cout << "Steiner points: " << this->get_all_steiner_nodes().size() << "\n";
}

template <> std::string visualize_routing_tree_svg(
    const GlobalRoutingTree<Point<int, int>>& tree,
    std::optional<std::vector<GlobalRoutingTree<Point<int, int>>::Keepout>> keepouts,
    const int width, const int height, const int margin) {
    std::vector<RoutingNode<Point<int, int>>*> all_nodes;
    all_nodes.reserve(tree.nodes.size());
    for (const auto& [node_id, routing_node] : tree.nodes) {
        all_nodes.push_back(routing_node);
    }
    if (all_nodes.empty()) {
        return "<svg></svg>";
    }

    auto params = detail::calculate_svg_params(all_nodes, width, height, margin);

    std::ostringstream svg;
    svg << "<svg width=\"" << width << "\" height=\"" << height
        << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    svg << "<defs>\n";
    svg << "<marker id=\"arrowhead\" markerWidth=\"10\" markerHeight=\"7\" refX=\"9\" refY=\"3.5\" "
           "orient=\"auto\">\n";
    svg << "<polygon points=\"0 0, 10 3.5, 0 7\" fill=\"black\"/>\n";
    svg << "</marker>\n";
    svg << "</defs>\n";

    std::function<void(const RoutingNode<Point<int, int>>*)> draw_connections =
        [&](const RoutingNode<Point<int, int>>* current_node) {
            for (auto* child : current_node->children) {
                auto [coord_x1, coord_y1] = detail::scale_coords(current_node->pt.xcoord(), current_node->pt.ycoord(), params);
                auto [coord_x2, coord_y2] = detail::scale_coords(child->pt.xcoord(), child->pt.ycoord(), params);
                svg << "<line x1=\"" << coord_x1 << "\" y1=\"" << coord_y1 << "\" x2=\"" << coord_x2 << "\" y2=\""
                    << coord_y2
                    << "\" stroke=\"black\" stroke-width=\"2\" marker-end=\"url(#arrowhead)\"/>\n";
                draw_connections(child);
            }
        };
    draw_connections(tree.get_source());

    if (keepouts.has_value()) {
        std::string color = "orange";
        for (const auto& keepout : *keepouts) {
            auto [keepout_x1, keepout_y1] = detail::scale_coords(keepout.xcoord().lb(), keepout.ycoord().lb(), params);
            auto [keepout_x2, keepout_y2] = detail::scale_coords(keepout.xcoord().ub(), keepout.ycoord().ub(), params);
            double rwidth = keepout_x2 - keepout_x1;
            double rheight = keepout_y2 - keepout_y1;
            svg << "<rect x=\"" << keepout_x1 << "\" y=\"" << keepout_y1 << "\" width=\"" << rwidth
                << "\" height=\"" << rheight << "\" fill=\"" << color
                << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
        }
    }

    for (auto* node : all_nodes) {
        detail::draw_node(svg, node, params);
    }

    detail::draw_legend(svg);
    detail::draw_stats(svg, tree);

    svg << "</svg>\n";
    return svg.str();
}

template <> void save_routing_tree_svg(
    const GlobalRoutingTree<Point<int, int>>& tree,
    std::optional<std::vector<GlobalRoutingTree<Point<int, int>>::Keepout>> keepouts,
    const std::string filename, const int width, const int height) {
    std::string svg_content = visualize_routing_tree_svg(tree, keepouts, width, height, 50);
    std::ofstream file_stream(filename);
    file_stream << svg_content;
    std::cout << "Routing tree saved to " << filename << "\n";
}

template <> std::string visualize_routing_tree3d_svg(
    const GlobalRoutingTree<Point<Point<int, int>, int>>& tree,
    std::optional<std::vector<GlobalRoutingTree<Point<Point<int, int>, int>>::Keepout>> keepouts,
    const int scale_z, const int width, const int height, const int margin) {
    std::vector<RoutingNode<Point<Point<int, int>, int>>*> all_nodes;
    all_nodes.reserve(tree.nodes.size());
    for (const auto& [node_id, routing_node] : tree.nodes) {
        all_nodes.push_back(routing_node);
    }
    if (all_nodes.empty()) {
        return "<svg></svg>";
    }

    auto params = detail::calculate_svg_params(all_nodes, width, height, margin);

    std::vector<std::string> layer_colors = {"red", "orange", "blue", "green"};

    std::ostringstream svg;
    svg << "<svg width=\"" << width << "\" height=\"" << height
        << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    svg << "<defs>\n";
    svg << "<marker id=\"arrowhead\" markerWidth=\"10\" markerHeight=\"7\" refX=\"9\" refY=\"3.5\" "
           "orient=\"auto\">\n";
    svg << "<polygon points=\"0 0, 10 3.5, 0 7\" fill=\"black\"/>\n";
    svg << "</marker>\n";
    svg << "</defs>\n";

    std::function<void(const RoutingNode<Point<Point<int, int>, int>>*)> draw_connections
        = [&](const RoutingNode<Point<Point<int, int>, int>>* current_node) {
              for (auto* child : current_node->children) {
                  auto [coord_x1, coord_y1] = detail::scale_coords(current_node->pt.xcoord().xcoord(), current_node->pt.ycoord(), params);
                  auto [coord_x2, coord_y2] = detail::scale_coords(child->pt.xcoord().xcoord(), child->pt.ycoord(), params);
                  size_t color_index = static_cast<size_t>(child->pt.xcoord().ycoord() / scale_z)
                                       % layer_colors.size();
                  std::string color = layer_colors[color_index];

                  svg << "<line x1=\"" << coord_x1 << "\" y1=\"" << coord_y1 << "\" x2=\"" << coord_x2 << "\" y2=\"" << coord_y2
                      << "\" stroke=\"" << color
                      << "\" stroke-width=\"2\" marker-end=\"url(#arrowhead)\"/>\n";
                  draw_connections(child);
              }
          };
    draw_connections(tree.get_source());

    if (keepouts.has_value()) {
        std::string color = "pink";
        for (const auto& keepout : *keepouts) {
            auto [keepout_x1, keepout_y1] = detail::scale_coords(keepout.xcoord().xcoord().lb(), keepout.ycoord().lb(), params);
            auto [keepout_x2, keepout_y2] = detail::scale_coords(keepout.xcoord().xcoord().ub(), keepout.ycoord().ub(), params);
            double rwidth = keepout_x2 - keepout_x1;
            double rheight = keepout_y2 - keepout_y1;
            svg << "<rect x=\"" << keepout_x1 << "\" y=\"" << keepout_y1 << "\" width=\"" << rwidth
                << "\" height=\"" << rheight << "\" fill=\"" << color
                << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
        }
    }

    for (auto* node : all_nodes) {
        detail::draw_node(svg, node, params);
    }

    detail::draw_legend(svg);
    detail::draw_stats(svg, tree);

    svg << "</svg>\n";
    return svg.str();
}

template <> void save_routing_tree3d_svg(
    const GlobalRoutingTree<Point<Point<int, int>, int>>& tree,
    std::optional<std::vector<GlobalRoutingTree<Point<Point<int, int>, int>>::Keepout>> keepouts,
    const int scale_z, const std::string filename, const int width, const int height) {
    std::string svg_content
        = visualize_routing_tree3d_svg(tree, keepouts, scale_z, width, height, 50);
    std::ofstream file_stream(filename);
    file_stream << svg_content;
    std::cout << "Routing tree (3d) saved to " << filename << "\n";
}