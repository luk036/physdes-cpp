// global_router.cpp
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

template <typename IntPoint>
std::ostream& operator<<(std::ostream& os, const RoutingNode<IntPoint>& node) {
    std::string type_name = to_string(node.type);
    os << type_name << "Node(" << node.id << ", (" << node.pt << "))";
    return os;
}

template <typename IntPoint>
auto GlobalRoutingTree<IntPoint>::get_tree_structure(const RoutingNode<IntPoint>* node,
                                                     int level) const -> std::string {
    if (node == nullptr) node = &source_node;
    std::ostringstream oss;
    oss << std::string(level * 2, ' ') << *node << "\n";
    for (auto child : node->children) {
        oss << get_tree_structure(child, level + 1);
    }
    return oss.str();
}

template <typename IntPoint> void GlobalRoutingTree<IntPoint>::visualize_tree() const {
    std::cout << "Global Routing Tree Structure:\n";
    std::cout << "========================================\n";
    std::cout << get_tree_structure();
    std::cout << "Total wirelength: " << calculate_wirelength() << "\n";
    std::cout << "Total nodes: " << nodes.size() << "\n";
    std::cout << "Terminals: " << get_all_terminals().size() << "\n";
    std::cout << "Steiner points: " << get_all_steiner_nodes().size() << "\n";
}

template <> std::string visualize_routing_tree_svg(
    const GlobalRoutingTree<Point<int, int>>& tree,
    std::optional<std::vector<GlobalRoutingTree<Point<int, int>>::Keepout>> keepouts,
    const int width, const int height, const int margin) {
    std::vector<RoutingNode<Point<int, int>>*> all_nodes;
    for (auto& [id, node] : tree.nodes) {
        all_nodes.push_back(node);
    }
    if (all_nodes.empty()) return "<svg></svg>";

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
        [&](const RoutingNode<Point<int, int>>* node) {
            for (auto child : node->children) {
                auto [x1, y1] = detail::scale_coords(node->pt.xcoord(), node->pt.ycoord(), params);
                auto [x2, y2] = detail::scale_coords(child->pt.xcoord(), child->pt.ycoord(), params);
                svg << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\""
                    << y2
                    << "\" stroke=\"black\" stroke-width=\"2\" marker-end=\"url(#arrowhead)\"/>\n";
                draw_connections(child);
            }
        };
    draw_connections(tree.get_source());

    if (keepouts.has_value()) {
        std::string color = "orange";
        for (const auto& keepout : *keepouts) {
            auto [x1, y1] = detail::scale_coords(keepout.xcoord().lb(), keepout.ycoord().lb(), params);
            auto [x2, y2] = detail::scale_coords(keepout.xcoord().ub(), keepout.ycoord().ub(), params);
            double rwidth = x2 - x1;
            double rheight = y2 - y1;
            svg << "<rect x=\"" << x1 << "\" y=\"" << y1 << "\" width=\"" << rwidth
                << "\" height=\"" << rheight << "\" fill=\"" << color
                << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
        }
    }

    for (auto node : all_nodes) {
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
    std::ofstream f(filename);
    f << svg_content;
    std::cout << "Routing tree saved to " << filename << "\n";
}

template <> std::string visualize_routing_tree3d_svg(
    const GlobalRoutingTree<Point<Point<int, int>, int>>& tree,
    std::optional<std::vector<GlobalRoutingTree<Point<Point<int, int>, int>>::Keepout>> keepouts,
    const int scale_z, const int width, const int height, const int margin) {
    std::vector<RoutingNode<Point<Point<int, int>, int>>*> all_nodes;
    for (auto& [id, node] : tree.nodes) {
        all_nodes.push_back(node);
    }
    if (all_nodes.empty()) return "<svg></svg>";

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
        = [&](const RoutingNode<Point<Point<int, int>, int>>* node) {
              for (auto child : node->children) {
                  auto [x1, y1] = detail::scale_coords(node->pt.xcoord().xcoord(), node->pt.ycoord(), params);
                  auto [x2, y2] = detail::scale_coords(child->pt.xcoord().xcoord(), child->pt.ycoord(), params);
                  size_t color_index = static_cast<size_t>(child->pt.xcoord().ycoord() / scale_z)
                                       % layer_colors.size();
                  std::string color = layer_colors[color_index];

                  svg << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\""
                      << y2 << "\" stroke=\"" << color
                      << "\" stroke-width=\"2\" marker-end=\"url(#arrowhead)\"/>\n";
                  draw_connections(child);
              }
          };
    draw_connections(tree.get_source());

    if (keepouts.has_value()) {
        std::string color = "pink";
        for (const auto& keepout : *keepouts) {
            auto [x1, y1] = detail::scale_coords(keepout.xcoord().xcoord().lb(), keepout.ycoord().lb(), params);
            auto [x2, y2] = detail::scale_coords(keepout.xcoord().xcoord().ub(), keepout.ycoord().ub(), params);
            double rwidth = x2 - x1;
            double rheight = y2 - y1;
            svg << "<rect x=\"" << x1 << "\" y=\"" << y1 << "\" width=\"" << rwidth
                << "\" height=\"" << rheight << "\" fill=\"" << color
                << "\" stroke=\"black\" stroke-width=\"1\"/>\n";
        }
    }

    for (auto node : all_nodes) {
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
    std::ofstream f(filename);
    f << svg_content;
    std::cout << "Routing tree (3d) saved to " << filename << "\n";
}
