// global_router.cpp - Implementation of global routing algorithms
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <recti/detail/svg_utils.hpp>
#include <recti/generic.hpp>
#include <recti/global_router.hpp>
// #include <recti/halton_int.hpp>
#include <recti/interval.hpp>
#include <recti/logger.hpp>
#include <recti/point.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace recti {

    // NodeType functions
    // =============================================================================

    std::string to_string(const NodeType routing_node_type) {
        switch (routing_node_type) {
            case NodeType::STEINER:
                return "Steiner";
            case NodeType::TERMINAL:
                return "Terminal";
            case NodeType::SOURCE:
                return "Source";
        }
        return "Unknown";
    }

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
    std::ostream& operator<<(std::ostream& output_stream,
                             const RoutingNode<IntPoint>& routing_node) {
        std::string type_name = to_string(routing_node.type);
        output_stream << type_name << "Node(" << routing_node.id << ", (" << routing_node.pt
                      << "))";
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

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::_find_nearest_insertion_with_constraints(
        const IntPoint& pt, int allowed_wirelength,
        std::optional<std::vector<GlobalRoutingTree<IntPoint>::Keepout>> keepouts)
        -> std::pair<RoutingNode<IntPoint>*, RoutingNode<IntPoint>*> {
        RoutingNode<IntPoint>* parent_node = nullptr;
        RoutingNode<IntPoint>* nearest_node = &this->source_node;
        int min_distance = this->worst_wirelength;
        // int min_distance = std::numeric_limits<int>::max();
        bool valid_found = false;

        std::function<void(RoutingNode<IntPoint>*)> traverse = [&](RoutingNode<IntPoint>* node) {
            for (auto* child : node->children) {
                auto possible_path = node->pt.hull_with(child->pt);
                auto distance = possible_path.min_dist_with(pt);
                auto nearest_pt = possible_path.nearest_to(pt);
                if (keepouts.has_value()) {
                    bool block = false;
                    auto path1 = nearest_pt.hull_with(pt);
                    auto path2 = nearest_pt.hull_with(node->pt);
                    auto path3 = nearest_pt.hull_with(child->pt);
                    for (const auto& keepout : *keepouts) {
                        if (keepout.contains(nearest_pt)) {
                            block = true;
                            break;
                        }
                        if (keepout.blocks(path1) || keepout.blocks(path2)
                            || keepout.blocks(path3)) {
                            block = true;
                            break;
                        }
                    }
                    if (block) {
                        continue;
                    }
                }
                int path_length = node->path_length + node->pt.min_dist_with(nearest_pt) + distance;
                bool update = false;
                if (path_length <= allowed_wirelength) {
                    if (valid_found) {
                        if (distance < min_distance) {
                            update = true;
                        }
                    } else {
                        valid_found = true;
                        update = true;
                    }
                } else {
                    if (!valid_found) {
                        // don't care allowed_wirelength if we haven't found any valid point yet
                        if (path_length <= this->worst_wirelength && distance < min_distance) {
                            update = true;
                        }
                    }
                }
                if (update) {
                    min_distance = distance;
                    if (nearest_pt == node->pt) {
                        nearest_node = node;
                        parent_node = nullptr;
                    } else if (nearest_pt == child->pt) {
                        nearest_node = child;
                        parent_node = nullptr;
                    } else {
                        parent_node = node;
                        nearest_node = child;
                    }
                }
                traverse(child);
            }
        };
        traverse(&this->source_node);
        if (!valid_found) {
            log_with_spdlog(
                "Warning: No valid insertion point found within allowed wirelength. "
                "Consider increasing the allowed wirelength or relaxing keepout constraints.");
        }
        return {parent_node, nearest_node};
    }

    template <typename IntPoint> auto GlobalRoutingTree<IntPoint>::_insert_terminal_impl(
        const IntPoint& point, int allowed_wirelength, std::optional<std::vector<Keepout>> keepouts)
        -> void {
        std::string terminal_id = "terminal_" + std::to_string(this->next_terminal_id++);
        auto terminal_ptr
            = std::make_unique<RoutingNode<IntPoint>>(terminal_id, NodeType::TERMINAL, point);
        RoutingNode<IntPoint>* terminal_node = terminal_ptr.get();
        this->nodes[terminal_id] = terminal_node;
        this->owned_nodes.emplace_back(std::move(terminal_ptr));
        auto [parent_node, nearest_node]
            = this->_find_nearest_insertion_with_constraints(point, allowed_wirelength, keepouts);
        if (parent_node == nullptr) {
            nearest_node->add_child(terminal_node);
            terminal_node->path_length
                = nearest_node->path_length + nearest_node->pt.min_dist_with(point);
        } else {
            std::string steiner_id = "steiner_" + std::to_string(this->next_steiner_id++);
            auto possible_path = parent_node->pt.hull_with(nearest_node->pt);
            IntPoint nearest_pt = possible_path.nearest_to(point);
            auto steiner_ptr = std::make_unique<RoutingNode<IntPoint>>(
                steiner_id, NodeType::STEINER, nearest_pt);
            RoutingNode<IntPoint>* new_node = steiner_ptr.get();
            this->nodes[steiner_id] = new_node;
            this->owned_nodes.emplace_back(std::move(steiner_ptr));
            parent_node->remove_child(nearest_node);
            parent_node->add_child(new_node);
            new_node->path_length
                = parent_node->path_length + parent_node->pt.min_dist_with(nearest_pt);
            new_node->add_child(nearest_node);
            new_node->add_child(terminal_node);
            terminal_node->path_length = new_node->path_length + nearest_pt.min_dist_with(point);
        }
    }

    template <typename IntPoint>
    GlobalRouter<IntPoint>::GlobalRouter(const IntPoint& source_pos,
                                         std::vector<IntPoint> terminal_positions,
                                         std::optional<std::vector<Keepout>> keepout_regions)
        : source_position(source_pos), tree(source_pos), keepouts(keepout_regions) {
        std::sort(terminal_positions.begin(), terminal_positions.end(),
                  [this](const IntPoint& point_a, const IntPoint& point_b) {
                      auto dist_a = this->source_position.min_dist_with(point_a);
                      auto dist_b = this->source_position.min_dist_with(point_b);
                      return dist_a < dist_b
                             || (dist_a == dist_b
                                 && this->source_position.hull_with(point_a).measure()
                                        > this->source_position.hull_with(point_b).measure());
                  });
        this->terminal_positions = std::move(terminal_positions);

        if (!this->terminal_positions.empty()) {
            this->worst_wirelength
                = this->source_position.min_dist_with(this->terminal_positions.back());
        } else {
            this->worst_wirelength = 0;
        }
    }

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::_find_nearest_node(const IntPoint& point,
                                                         std::optional<std::string> exclude_id)
        -> RoutingNode<IntPoint>* {
        if (this->nodes.size() <= 1) return &this->source_node;
        RoutingNode<IntPoint>* nearest = &this->source_node;
        int min_dist = this->source_node.pt.min_dist_with(point);
        for (auto& [id, node] : this->nodes) {
            if (exclude_id && id == *exclude_id) continue;
            int distance = node->pt.min_dist_with(point);
            if (distance < min_dist) {
                min_dist = distance;
                nearest = node;
            }
        }
        return nearest;
    }

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::insert_steiner_node(const IntPoint& point,
                                                          std::optional<std::string> parent_id)
        -> std::string {
        std::string steiner_id = "steiner_" + std::to_string(this->next_steiner_id++);
        auto node_ptr
            = std::make_unique<RoutingNode<IntPoint>>(steiner_id, NodeType::STEINER, point);
        RoutingNode<IntPoint>* node = node_ptr.get();
        this->nodes[steiner_id] = node;
        this->owned_nodes.emplace_back(std::move(node_ptr));

        RoutingNode<IntPoint>* parent_node;
        if (!parent_id) {
            parent_node = &this->source_node;
        } else {
            auto iter = this->nodes.find(*parent_id);
            if (iter == this->nodes.end()) {
                throw std::runtime_error("Parent node " + *parent_id + " not found");
            }
            parent_node = iter->second;
        }
        parent_node->add_child(node);
        return steiner_id;
    }

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::insert_terminal_node(const IntPoint& point,
                                                           std::optional<std::string> parent_id)
        -> std::string {
        std::string terminal_id = "terminal_" + std::to_string(this->next_terminal_id++);
        auto node_ptr
            = std::make_unique<RoutingNode<IntPoint>>(terminal_id, NodeType::TERMINAL, point);
        RoutingNode<IntPoint>* node = node_ptr.get();
        this->nodes[terminal_id] = node;
        this->owned_nodes.emplace_back(std::move(node_ptr));

        RoutingNode<IntPoint>* parent_node;
        if (!parent_id) {
            parent_node = this->_find_nearest_node(point);
        } else {
            auto iter = this->nodes.find(*parent_id);
            if (iter == this->nodes.end()) {
                throw std::runtime_error("Parent node " + *parent_id + " not found");
            }
            parent_node = iter->second;
        }
        parent_node->add_child(node);
        return terminal_id;
    }

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::insert_node_on_branch(NodeType new_node_type,
                                                            const IntPoint& point,
                                                            std::string branch_start_id,
                                                            std::string branch_end_id)
        -> std::string {
        auto start_iter = this->nodes.find(branch_start_id);
        auto end_iter = this->nodes.find(branch_end_id);
        if (start_iter == this->nodes.end() || end_iter == this->nodes.end()) {
            throw std::runtime_error("One or both branch nodes not found");
        }
        RoutingNode<IntPoint>* start_node = start_iter->second;
        RoutingNode<IntPoint>* end_node = end_iter->second;
        auto child_iter
            = std::find(start_node->children.begin(), start_node->children.end(), end_node);
        if (child_iter == start_node->children.end()) {
            throw std::runtime_error(branch_end_id + " is not a direct child of "
                                     + branch_start_id);
        }

        std::string node_id;
        if (new_node_type == NodeType::STEINER) {
            node_id = "steiner_" + std::to_string(this->next_steiner_id++);
        } else if (new_node_type == NodeType::TERMINAL) {
            node_id = "terminal_" + std::to_string(this->next_terminal_id++);
        }
        auto node_ptr = std::make_unique<RoutingNode<IntPoint>>(node_id, new_node_type, point);
        RoutingNode<IntPoint>* new_node = node_ptr.get();
        this->nodes[node_id] = new_node;
        this->owned_nodes.emplace_back(std::move(node_ptr));

        start_node->remove_child(end_node);
        start_node->add_child(new_node);
        new_node->add_child(end_node);
        return node_id;
    }

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::calculate_total_wirelength() const -> int {
        int total = 0;
        std::function<void(const RoutingNode<IntPoint>*)> traverse
            = [&](const RoutingNode<IntPoint>* current_node) -> void {
            for (auto child : current_node->children) {
                total += current_node->manhattan_distance(child);
                traverse(child);
            }
        };
        traverse(&source_node);
        return total;
    }

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::calculate_worst_wirelength() const -> int {
        int worst_length = 0;
        std::function<int(const RoutingNode<IntPoint>*)> traverse
            = [&](const RoutingNode<IntPoint>* current_node) -> int {
            for (auto child : current_node->children) {
                auto length = traverse(child);
                worst_length
                    = std::max(worst_length, length + current_node->manhattan_distance(child));
            }
            return worst_length;
        };

        auto length = traverse(&source_node);
        return length;
    }

    template <typename IntPoint>
    auto GlobalRoutingTree<IntPoint>::find_path_to_source(const std::string& node_id) const
        -> std::vector<const RoutingNode<IntPoint>*> {
        auto iter = nodes.find(node_id);
        if (iter == nodes.end()) {
            throw std::runtime_error("Node " + node_id + " not found");
        }
        const RoutingNode<IntPoint>* current = iter->second;
        std::vector<const RoutingNode<IntPoint>*> path;
        while (current) {
            path.emplace_back(current);
            current = current->parent;
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

    template <typename IntPoint> auto GlobalRoutingTree<IntPoint>::get_all_terminals() const
        -> std::vector<const RoutingNode<IntPoint>*> {
        std::vector<const RoutingNode<IntPoint>*> terms;
        for (auto& pair : this->nodes) {
            const auto& node = pair.second;
            if (node->type == NodeType::TERMINAL) {
                terms.emplace_back(node);
            }
        }
        return terms;
    }

    template <typename IntPoint> auto GlobalRoutingTree<IntPoint>::get_all_steiner_nodes() const
        -> std::vector<const RoutingNode<IntPoint>*> {
        std::vector<const RoutingNode<IntPoint>*> steins;
        for (auto& pair : this->nodes) {
            const auto& node = pair.second;
            if (node->type == NodeType::STEINER) {
                steins.emplace_back(node);
            }
        }
        return steins;
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
        std::cout << "Total wirelength: " << this->calculate_total_wirelength() << "\n";
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
            all_nodes.emplace_back(routing_node);
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
        svg << "<marker id=\"arrowhead\" markerWidth=\"10\" markerHeight=\"7\" refX=\"9\" "
               "refY=\"3.5\" "
               "orient=\"auto\">\n";
        svg << "<polygon points=\"0 0, 10 3.5, 0 7\" fill=\"black\"/>\n";
        svg << "</marker>\n";
        svg << "</defs>\n";

        std::function<void(const RoutingNode<Point<int, int>>*)> draw_connections
            = [&](const RoutingNode<Point<int, int>>* current_node) {
                  for (auto* child : current_node->children) {
                      auto [coord_x1, coord_y1] = detail::scale_coords(
                          current_node->pt.xcoord(), current_node->pt.ycoord(), params);
                      auto [coord_x2, coord_y2]
                          = detail::scale_coords(child->pt.xcoord(), child->pt.ycoord(), params);
                      svg << "<line x1=\"" << coord_x1 << "\" y1=\"" << coord_y1 << "\" x2=\""
                          << coord_x2 << "\" y2=\"" << coord_y2
                          << "\" stroke=\"black\" stroke-width=\"2\" "
                             "marker-end=\"url(#arrowhead)\"/>\n";
                      draw_connections(child);
                  }
              };
        draw_connections(tree.get_source());

        if (keepouts.has_value()) {
            std::string color = "orange";
            for (const auto& keepout : *keepouts) {
                auto [keepout_x1, keepout_y1]
                    = detail::scale_coords(keepout.xcoord().lb(), keepout.ycoord().lb(), params);
                auto [keepout_x2, keepout_y2]
                    = detail::scale_coords(keepout.xcoord().ub(), keepout.ycoord().ub(), params);
                double rwidth = keepout_x2 - keepout_x1;
                double rheight = keepout_y2 - keepout_y1;
                svg << "<rect x=\"" << keepout_x1 << "\" y=\"" << keepout_y1 << "\" width=\""
                    << rwidth << "\" height=\"" << rheight << "\" fill=\"" << color
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
        std::optional<std::vector<GlobalRoutingTree<Point<Point<int, int>, int>>::Keepout>>
            keepouts,
        const int scale_z, const int width, const int height, const int margin) {
        std::vector<RoutingNode<Point<Point<int, int>, int>>*> all_nodes;
        all_nodes.reserve(tree.nodes.size());
        for (const auto& [node_id, routing_node] : tree.nodes) {
            all_nodes.emplace_back(routing_node);
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
        svg << "<marker id=\"arrowhead\" markerWidth=\"10\" markerHeight=\"7\" refX=\"9\" "
               "refY=\"3.5\" "
               "orient=\"auto\">\n";
        svg << "<polygon points=\"0 0, 10 3.5, 0 7\" fill=\"black\"/>\n";
        svg << "</marker>\n";
        svg << "</defs>\n";

        std::function<void(const RoutingNode<Point<Point<int, int>, int>>*)> draw_connections =
            [&](const RoutingNode<Point<Point<int, int>, int>>* current_node) {
                for (auto* child : current_node->children) {
                    auto [coord_x1, coord_y1] = detail::scale_coords(
                        current_node->pt.xcoord().xcoord(), current_node->pt.ycoord(), params);
                    auto [coord_x2, coord_y2] = detail::scale_coords(child->pt.xcoord().xcoord(),
                                                                     child->pt.ycoord(), params);
                    size_t color_index = static_cast<size_t>(child->pt.xcoord().ycoord() / scale_z)
                                         % layer_colors.size();
                    std::string color = layer_colors[color_index];

                    svg << "<line x1=\"" << coord_x1 << "\" y1=\"" << coord_y1 << "\" x2=\""
                        << coord_x2 << "\" y2=\"" << coord_y2 << "\" stroke=\"" << color
                        << "\" stroke-width=\"2\" marker-end=\"url(#arrowhead)\"/>\n";
                    draw_connections(child);
                }
            };
        draw_connections(tree.get_source());

        if (keepouts.has_value()) {
            std::string color = "pink";
            for (const auto& keepout : *keepouts) {
                auto [keepout_x1, keepout_y1] = detail::scale_coords(keepout.xcoord().xcoord().lb(),
                                                                     keepout.ycoord().lb(), params);
                auto [keepout_x2, keepout_y2] = detail::scale_coords(keepout.xcoord().xcoord().ub(),
                                                                     keepout.ycoord().ub(), params);
                double rwidth = keepout_x2 - keepout_x1;
                double rheight = keepout_y2 - keepout_y1;
                svg << "<rect x=\"" << keepout_x1 << "\" y=\"" << keepout_y1 << "\" width=\""
                    << rwidth << "\" height=\"" << rheight << "\" fill=\"" << color
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
        std::optional<std::vector<GlobalRoutingTree<Point<Point<int, int>, int>>::Keepout>>
            keepouts,
        const int scale_z, const std::string filename, const int width, const int height) {
        std::string svg_content
            = visualize_routing_tree3d_svg(tree, keepouts, scale_z, width, height, 50);
        std::ofstream file_stream(filename);
        file_stream << svg_content;
        std::cout << "Routing tree (3d) saved to " << filename << "\n";
    }

    template <typename IntPoint> void GlobalRoutingTree<IntPoint>::optimize_steiner_points() {
        std::vector<std::string> steiner_ids_to_remove;
        for (auto& [id, node] : this->nodes) {
            if (node->type == NodeType::STEINER && node->children.size() == 1
                && node->parent != nullptr) {
                steiner_ids_to_remove.emplace_back(id);
            }
        }

        for (const auto& steiner_id : steiner_ids_to_remove) {
            RoutingNode<IntPoint>* steiner = this->nodes.at(steiner_id);
            RoutingNode<IntPoint>* parent = steiner->parent;
            RoutingNode<IntPoint>* child = steiner->children[0];

            parent->remove_child(steiner);
            parent->add_child(child);

            this->nodes.erase(steiner_id);
            auto owned_iter
                = std::find_if(this->owned_nodes.begin(), this->owned_nodes.end(),
                               [steiner](const auto& up) { return up.get() == steiner; });
            if (owned_iter != this->owned_nodes.end()) {
                this->owned_nodes.erase(owned_iter);
            }
        }
    }

    template class GlobalRoutingTree<Point<int, int>>;
    template class GlobalRoutingTree<Point<Point<int, int>, int>>;
    template class GlobalRouter<Point<int, int>>;
    template class GlobalRouter<Point<Point<int, int>, int>>;

}  // namespace recti

namespace recti::detail {

    template <> SvgParams calculate_svg_params<Point<int, int>>(
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

    template <> SvgParams calculate_svg_params<Point<Point<int, int>, int>>(
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

    // std::pair<double, double> scale_coords(int x, int y, const SvgParams& params) {
    //     double sx = params.margin + (x - params.min_x) * params.scale;
    //     double sy = params.margin + (y - params.min_y) * params.scale;
    //     return {sx, sy};
    // }

    template <> void draw_node<Point<int, int>>(std::ostringstream& svg,
                                                const RoutingNode<Point<int, int>>* node,
                                                const SvgParams& params) {
        auto [x, y] = scale_coords(node->pt.xcoord(), node->pt.ycoord(), params);
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
            label = "S";
            label += pos != std::string::npos ? node->id.substr(pos + 1) : std::string("");
        } else if (node->type == NodeType::TERMINAL) {
            color = "green";
            radius = 6;
            size_t pos = node->id.find('_');
            label = "T";
            label += pos != std::string::npos ? node->id.substr(pos + 1) : std::string("");
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

    template <> void draw_node<Point<Point<int, int>, int>>(
        std::ostringstream& svg, const RoutingNode<Point<Point<int, int>, int>>* node,
        const SvgParams& params) {
        auto [x, y] = scale_coords(node->pt.xcoord().xcoord(), node->pt.ycoord(), params);
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
            label = "S";
            label += pos != std::string::npos ? std::string(node->id.substr(pos + 1))
                                              : std::string("");
        } else if (node->type == NodeType::TERMINAL) {
            color = "green";
            radius = 6;
            size_t pos = node->id.find('_');
            label = "T";
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

    void draw_legend(std::ostringstream& svg) {
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

    template <> void draw_stats<Point<int, int>>(std::ostringstream& svg,
                                                 const GlobalRoutingTree<Point<int, int>>& tree) {
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
            << tree.calculate_total_wirelength() << "</text>\n";
    }

    template <> void draw_stats<Point<Point<int, int>, int>>(
        std::ostringstream& svg, const GlobalRoutingTree<Point<Point<int, int>, int>>& tree) {
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
            << tree.calculate_total_wirelength() << "</text>\n";
    }

}  // namespace recti::detail