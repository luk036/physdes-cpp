#include <algorithm>
#include <fstream>
#include <iostream>
#include <recti/generic.hpp>
#include <recti/global_router.hpp>
#include <recti/halton_int.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// using recti::nearest_to;

std::ostream& operator<<(std::ostream& os, const RoutingNode& node) {
    std::string type_name = to_string(node.type);
    os << type_name << "Node(" << node.id << ", (" << node.pt.xcoord() << ", " << node.pt.ycoord()
       << "))";
    return os;
}

void GlobalRoutingTree::visualize_tree() const {
    std::cout << "Global Routing Tree Structure:\n";
    std::cout << "========================================\n";
    std::cout << get_tree_structure();
    std::cout << "Total wirelength: " << calculate_wirelength() << "\n";
    std::cout << "Total nodes: " << nodes.size() << "\n";
    std::cout << "Terminals: " << get_all_terminals().size() << "\n";
    std::cout << "Steiner points: " << get_all_steiner_nodes().size() << "\n";
}

auto GlobalRoutingTree::get_tree_structure(const RoutingNode* node, int level) const
    -> std::string {
    if (node == nullptr) node = &source_node;
    std::ostringstream oss;
    oss << std::string(level * 2, ' ') << *node << "\n";
    for (auto child : node->children) {
        oss << get_tree_structure(child, level + 1);
    }
    return oss.str();
}

std::string visualize_routing_tree_svg(const GlobalRoutingTree& tree, const int width,
                                       const int height, const int margin) {
    std::vector<RoutingNode*> all_nodes;
    for (auto& [id, node] : tree.nodes) {
        all_nodes.push_back(node);
    }
    if (all_nodes.empty()) return "<svg></svg>";

    int min_x = all_nodes[0]->pt.xcoord(), max_x = min_x;
    int min_y = all_nodes[0]->pt.ycoord(), max_y = min_y;
    for (auto node : all_nodes) {
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

    auto scale_coords = [&](int x, int y) -> std::pair<double, double> {
        double sx = margin + (x - min_x) * scale;
        double sy = margin + (y - min_y) * scale;
        return {sx, sy};
    };

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

    std::function<void(const RoutingNode*)> draw_connections = [&](const RoutingNode* node) {
        for (auto child : node->children) {
            auto [x1, y1] = scale_coords(node->pt.xcoord(), node->pt.ycoord());
            auto [x2, y2] = scale_coords(child->pt.xcoord(), child->pt.ycoord());
            svg << "<line x1=\"" << x1 << "\" y1=\"" << y1 << "\" x2=\"" << x2 << "\" y2=\"" << y2
                << "\" stroke=\"black\" stroke-width=\"2\" marker-end=\"url(#arrowhead)\"/>\n";
            draw_connections(child);
        }
    };
    draw_connections(tree.get_source());

    for (auto node : all_nodes) {
        auto [x, y] = scale_coords(node->pt.xcoord(), node->pt.ycoord());
        std::string color;
        int radius;
        std::string label;
        if (node->type == NodeType::SOURCE) {
            color = "red";
            radius = 8;
            label = "S";
        } else if (node->type == NodeType::STEINER) {
            color = "blue";
            radius = 6;
            size_t pos = node->id.find('_');
            label = "S" + (pos != std::string::npos ? node->id.substr(pos + 1) : "");
        } else if (node->type == NodeType::TERMINAL) {
            color = "green";
            radius = 6;
            size_t pos = node->id.find('_');
            label = "T" + (pos != std::string::npos ? node->id.substr(pos + 1) : "");
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
            << node->pt.xcoord() << "," << node->pt.ycoord() << ")</text>\n";
    }

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

    int stats_y = legend_y + 90;
    svg << "<text x=\"20\" y=\"" << stats_y
        << "\" font-family=\"Arial\" font-size=\"10\" font-weight=\"bold\">Statistics:</text>\n";
    svg << "<text x=\"20\" y=\"" << stats_y + 15
        << "\" font-family=\"Arial\" font-size=\"9\">Total Nodes: " << tree.nodes.size()
        << "</text>\n";
    svg << "<text x=\"20\" y=\"" << stats_y + 30
        << "\" font-family=\"Arial\" font-size=\"9\">Terminals: " << tree.get_all_terminals().size()
        << "</text>\n";
    svg << "<text x=\"20\" y=\"" << stats_y + 45
        << "\" font-family=\"Arial\" font-size=\"9\">Steiner: "
        << tree.get_all_steiner_nodes().size() << "</text>\n";
    svg << "<text x=\"20\" y=\"" << stats_y + 60
        << "\" font-family=\"Arial\" font-size=\"9\">Wirelength: " << tree.calculate_wirelength()
        << "</text>\n";

    svg << "</svg>\n";
    return svg.str();
}

void save_routing_tree_svg(const GlobalRoutingTree& tree, const std::string filename,
                           const int width, const int height) {
    std::string svg_content = visualize_routing_tree_svg(tree, width, height);
    std::ofstream f(filename);
    f << svg_content;
    std::cout << "Routing tree saved to " << filename << "\n";
}

// void test_route_with_steiner() {
//     unsigned base[] = {3, 2};
//     unsigned scale[] = {7, 11};
//     auto hgen = recti::halton(base, scale);
//     // Assuming no reseed, adjust if halton has reseed
//     std::vector<std::vector<unsigned>> coords;
//     for (int i = 0; i < 7; ++i) {
//         coords.push_back(hgen());
//     }
//     std::vector<IntPoint> terminals;
//     for (const auto& coord : coords) {
//         terminals.emplace_back(static_cast<int>(coord[0]), static_cast<int>(coord[1]));
//     }
//     auto src_coord = hgen();
//     IntPoint source(static_cast<int>(src_coord[0]), static_cast<int>(src_coord[1]));

//     GlobalRouter router(source, terminals);
//     router.route_with_steiners();

//     std::string svg_output = visualize_routing_tree_svg(router.get_tree(), 1000, 1000);
//     std::cout << svg_output;

//     save_routing_tree_svg(router.get_tree(), "example_route_with_steiner.svg");
// }

// void test_route_with_constraints() {
//     unsigned base[] = {3, 2};
//     unsigned scale[] = {7, 11};
//     auto hgen = recti::halton(base, scale);
//     // Assuming no reseed
//     std::vector<std::vector<unsigned>> coords;
//     for (int i = 0; i < 7; ++i) {
//         coords.push_back(hgen());
//     }
//     std::vector<IntPoint> terminals;
//     for (const auto& coord : coords) {
//         terminals.emplace_back(static_cast<int>(coord[0]), static_cast<int>(coord[1]));
//     }
//     auto src_coord = hgen();
//     IntPoint source(static_cast<int>(src_coord[0]), static_cast<int>(src_coord[1]));

//     GlobalRouter router(source, terminals);
//     router.route_with_constraints(0.9);

//     std::string svg_output = visualize_routing_tree_svg(router.get_tree(), 1000, 1000);
//     std::cout << svg_output;

//     save_routing_tree_svg(router.get_tree(), "example_route_with_constraint.svg");
// }