#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <recti/generic.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace recti {

    using IntPoint = Point<int>;
    using IntRect = Point<Interval<int>, Interval<int>>;

}  // namespace recti

using recti::IntPoint;
using recti::IntRect;

enum class NodeType { STEINER, TERMINAL, SOURCE };

inline std::string to_string(NodeType t) {
    switch (t) {
        case NodeType::STEINER:
            return "Steiner";
        case NodeType::TERMINAL:
            return "Terminal";
        case NodeType::SOURCE:
            return "Source";
    }
    return "Unknown";
}

class RoutingNode {
  public:
    std::string id;
    NodeType type;
    IntPoint pt;
    std::vector<RoutingNode*> children;
    RoutingNode* parent = nullptr;
    double capacitance = 0.0;
    double delay = 0.0;
    int path_length = 0;

    RoutingNode(std::string id_, NodeType type_, IntPoint pt_ = {0, 0})
        : id(std::move(id_)), type(type_), pt(pt_) {}

    void add_child(RoutingNode* child) {
        child->parent = this;
        children.push_back(child);
    }

    void remove_child(RoutingNode* child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end()) {
            children.erase(it);
            child->parent = nullptr;
        }
    }

    auto get_position() const -> const IntPoint& { return pt; }

    auto manhattan_distance(const RoutingNode* other) const -> int {
        return pt.min_dist_with(other->pt);
    }
};

class GlobalRoutingTree {
    RoutingNode source_node;
    int next_steiner_id = 1;
    int next_terminal_id = 1;

  public:
    std::unordered_map<std::string, RoutingNode*> nodes;
    std::vector<std::unique_ptr<RoutingNode>> owned_nodes;

  public:
    GlobalRoutingTree(IntPoint source_position = {0, 0})
        : source_node("source", NodeType::SOURCE, source_position) {
        nodes["source"] = &source_node;
    }

    auto get_source() const -> const RoutingNode* { return &source_node; }

    auto get_source() -> RoutingNode* { return &source_node; }

    auto insert_steiner_node(IntPoint pt, std::optional<std::string> parent_id = std::nullopt)
        -> std::string {
        std::string steiner_id = "steiner_" + std::to_string(next_steiner_id++);
        auto node_ptr = std::make_unique<RoutingNode>(steiner_id, NodeType::STEINER, pt);
        RoutingNode* node = node_ptr.get();
        nodes[steiner_id] = node;
        owned_nodes.push_back(std::move(node_ptr));

        RoutingNode* parent_node;
        if (!parent_id) {
            parent_node = &source_node;
        } else {
            auto it = nodes.find(*parent_id);
            if (it == nodes.end()) {
                throw std::runtime_error("Parent node " + *parent_id + " not found");
            }
            parent_node = it->second;
        }
        parent_node->add_child(node);
        return steiner_id;
    }

    auto _find_nearest_node(IntPoint pt) -> RoutingNode* {
        if (nodes.size() <= 1) return &source_node;
        RoutingNode temp("temp", NodeType::STEINER, pt);
        RoutingNode* nearest = &source_node;
        int min_dist = source_node.manhattan_distance(&temp);
        for (auto& [id, node] : nodes) {
            int dist = node->manhattan_distance(&temp);
            if (dist < min_dist) {
                min_dist = dist;
                nearest = node;
            }
        }
        return nearest;
    }

    auto insert_terminal_node(IntPoint pt, std::optional<std::string> parent_id = std::nullopt)
        -> std::string {
        std::string terminal_id = "terminal_" + std::to_string(next_terminal_id++);
        auto node_ptr = std::make_unique<RoutingNode>(terminal_id, NodeType::TERMINAL, pt);
        RoutingNode* node = node_ptr.get();
        nodes[terminal_id] = node;
        owned_nodes.push_back(std::move(node_ptr));

        RoutingNode* parent_node;
        if (!parent_id) {
            parent_node = _find_nearest_node(pt);
        } else {
            auto it = nodes.find(*parent_id);
            if (it == nodes.end()) {
                throw std::runtime_error("Parent node " + *parent_id + " not found");
            }
            parent_node = it->second;
        }
        parent_node->add_child(node);
        return terminal_id;
    }

    auto insert_node_on_branch(NodeType new_node_type, int x, int y, std::string branch_start_id,
                               std::string branch_end_id) -> std::string {
        auto start_it = nodes.find(branch_start_id);
        auto end_it = nodes.find(branch_end_id);
        if (start_it == nodes.end() || end_it == nodes.end()) {
            throw std::runtime_error("One or both branch nodes not found");
        }
        RoutingNode* start_node = start_it->second;
        RoutingNode* end_node = end_it->second;
        auto child_it
            = std::find(start_node->children.begin(), start_node->children.end(), end_node);
        if (child_it == start_node->children.end()) {
            throw std::runtime_error(branch_end_id + " is not a direct child of "
                                     + branch_start_id);
        }

        std::string node_id;
        if (new_node_type == NodeType::STEINER) {
            node_id = "steiner_" + std::to_string(next_steiner_id++);
        } else if (new_node_type == NodeType::TERMINAL) {
            node_id = "terminal_" + std::to_string(next_terminal_id++);
        } else {
            throw std::runtime_error("Node type must be STEINER or TERMINAL");
        }

        auto node_ptr = std::make_unique<RoutingNode>(node_id, new_node_type, IntPoint(x, y));
        RoutingNode* new_node = node_ptr.get();
        nodes[node_id] = new_node;
        owned_nodes.push_back(std::move(node_ptr));

        start_node->remove_child(end_node);
        start_node->add_child(new_node);
        new_node->add_child(end_node);
        return node_id;
    }

    using InsertionPair = std::pair<RoutingNode*, RoutingNode*>;
    auto _find_nearest_insertion(IntPoint pt) -> InsertionPair {
        if (nodes.size() <= 1) return {nullptr, &source_node};
        RoutingNode* nearest_node = &source_node;
        RoutingNode* parent_node = nullptr;
        int min_distance = source_node.pt.min_dist_with(pt);
        std::function<void(RoutingNode*)> traverse = [&](RoutingNode* node) {
            for (auto child : node->children) {
                IntRect possible_path = node->pt.hull_with(child->pt);
                int distance = possible_path.min_dist_with(pt);
                if (distance < min_distance) {
                    min_distance = distance;
                    IntPoint nearest_pt = possible_path.nearest_to(pt);
                    if (nearest_pt == node->pt) {
                        nearest_node = node;
                        parent_node = nullptr;
                    } else if (nearest_pt == child->pt) {
                        nearest_node = child;
                        parent_node = nullptr;
                    } else {
                        nearest_node = child;
                        parent_node = node;
                    }
                }
                traverse(child);
            }
        };
        traverse(&source_node);
        return {parent_node, nearest_node};
    }

    void insert_terminal_with_steiner(IntPoint pt) {
        std::string terminal_id = "terminal_" + std::to_string(next_terminal_id++);
        auto node_ptr = std::make_unique<RoutingNode>(terminal_id, NodeType::TERMINAL, pt);
        RoutingNode* terminal_node = node_ptr.get();
        owned_nodes.push_back(std::move(node_ptr));
        nodes[terminal_id] = terminal_node;

        auto [parent_node, nearest_node] = _find_nearest_insertion(pt);
        if (parent_node == nullptr) {
            nearest_node->add_child(terminal_node);
        } else {
            std::string steiner_id = "steiner_" + std::to_string(next_steiner_id++);
            IntRect possible_path = parent_node->pt.hull_with(nearest_node->pt);
            IntPoint nearest_pt = possible_path.nearest_to(pt);
            auto steiner_ptr
                = std::make_unique<RoutingNode>(steiner_id, NodeType::STEINER, nearest_pt);
            RoutingNode* new_node = steiner_ptr.get();
            nodes[steiner_id] = new_node;
            owned_nodes.push_back(std::move(steiner_ptr));

            parent_node->remove_child(nearest_node);
            parent_node->add_child(new_node);
            new_node->add_child(nearest_node);
            new_node->add_child(terminal_node);
        }
    }

    auto _find_nearest_insertion_with_constraints(IntPoint pt, int allowed_wirelength)
        -> InsertionPair {
        if (nodes.size() <= 1) return {nullptr, &source_node};
        RoutingNode* nearest_node = &source_node;
        RoutingNode* parent_node = nullptr;
        int min_distance = source_node.pt.min_dist_with(pt);
        std::function<void(RoutingNode*)> traverse = [&](RoutingNode* node) {
            for (auto child : node->children) {
                IntRect possible_path = node->pt.hull_with(child->pt);
                int distance = possible_path.min_dist_with(pt);
                IntPoint nearest_pt = possible_path.nearest_to(pt);
                int path_length = node->path_length + node->pt.min_dist_with(nearest_pt) + distance;
                if (path_length > allowed_wirelength) continue;
                if (distance < min_distance) {
                    min_distance = distance;
                    if (nearest_pt == node->pt) {
                        nearest_node = node;
                        parent_node = nullptr;
                    } else if (nearest_pt == child->pt) {
                        nearest_node = child;
                        parent_node = nullptr;
                    } else {
                        nearest_node = child;
                        parent_node = node;
                    }
                }
                traverse(child);
            }
        };
        traverse(&source_node);
        return {parent_node, nearest_node};
    }

    void insert_terminal_with_constraints(IntPoint pt, int allowed_wirelength) {
        std::string terminal_id = "terminal_" + std::to_string(next_terminal_id++);
        auto node_ptr = std::make_unique<RoutingNode>(terminal_id, NodeType::TERMINAL, pt);
        RoutingNode* terminal_node = node_ptr.get();
        owned_nodes.push_back(std::move(node_ptr));
        nodes[terminal_id] = terminal_node;

        auto [parent_node, nearest_node]
            = _find_nearest_insertion_with_constraints(pt, allowed_wirelength);
        if (parent_node == nullptr) {
            nearest_node->add_child(terminal_node);
            terminal_node->path_length
                = nearest_node->path_length + nearest_node->pt.min_dist_with(pt);
        } else {
            std::string steiner_id = "steiner_" + std::to_string(next_steiner_id++);
            IntRect possible_path = parent_node->pt.hull_with(nearest_node->pt);
            IntPoint nearest_pt = possible_path.nearest_to(pt);
            auto steiner_ptr
                = std::make_unique<RoutingNode>(steiner_id, NodeType::STEINER, nearest_pt);
            RoutingNode* new_node = steiner_ptr.get();
            nodes[steiner_id] = new_node;
            owned_nodes.push_back(std::move(steiner_ptr));

            parent_node->remove_child(nearest_node);
            parent_node->add_child(new_node);
            new_node->path_length
                = parent_node->path_length + parent_node->pt.min_dist_with(nearest_pt);
            new_node->add_child(nearest_node);
            new_node->add_child(terminal_node);
            terminal_node->path_length = new_node->path_length + nearest_pt.min_dist_with(pt);
        }
    }

    auto calculate_wirelength() const -> int {
        int total = 0;
        std::function<void(const RoutingNode*)> traverse = [&](const RoutingNode* node) {
            for (auto child : node->children) {
                total += node->manhattan_distance(child);
                traverse(child);
            }
        };
        traverse(&source_node);
        return total;
    }

    auto get_tree_structure(const RoutingNode* node = nullptr, int level = 0) const -> std::string;

    auto find_path_to_source(std::string node_id) const -> std::vector<const RoutingNode*> {
        auto it = nodes.find(node_id);
        if (it == nodes.end()) {
            throw std::runtime_error("Node " + node_id + " not found");
        }
        const RoutingNode* current = it->second;
        std::vector<const RoutingNode*> path;
        while (current) {
            path.push_back(current);
            current = current->parent;
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

    auto get_all_terminals() const -> std::vector<RoutingNode*> {
        std::vector<RoutingNode*> terms;
        for (auto& [id, node] : nodes) {
            if (node->type == NodeType::TERMINAL) {
                terms.push_back(node);
            }
        }
        return terms;
    }

    auto get_all_steiner_nodes() const -> std::vector<RoutingNode*> {
        std::vector<RoutingNode*> steins;
        for (auto& [id, node] : nodes) {
            if (node->type == NodeType::STEINER) {
                steins.push_back(node);
            }
        }
        return steins;
    }

    void optimize_steiner_points() {
        auto steiners = get_all_steiner_nodes();
        for (auto steiner : steiners) {
            if (steiner->children.size() == 1 && steiner->parent != nullptr) {
                auto parent = steiner->parent;
                auto child = steiner->children[0];
                parent->remove_child(steiner);
                parent->add_child(child);
                nodes.erase(steiner->id);
                auto owned_it
                    = std::find_if(owned_nodes.begin(), owned_nodes.end(),
                                   [steiner](const auto& up) { return up.get() == steiner; });
                if (owned_it != owned_nodes.end()) {
                    owned_nodes.erase(owned_it);
                }
            }
        }
    }

    void visualize_tree() const;
};

class GlobalRouter {
    IntPoint source_position;
    std::vector<IntPoint> terminal_positions;
    GlobalRoutingTree tree;
    int worst_wirelength = 0;

  public:
    GlobalRouter(IntPoint source_, std::vector<IntPoint> terminals)
        : source_position(source_), tree(source_) {
        terminal_positions = terminals;
        if (!terminal_positions.empty()) {
            worst_wirelength = source_position.min_dist_with(terminal_positions[0]);
            for (const auto& t : terminal_positions) {
                worst_wirelength = std::max(worst_wirelength, source_position.min_dist_with(t));
            }
        }
        std::sort(terminal_positions.begin(), terminal_positions.end(),
                  [this](const IntPoint& a, const IntPoint& b) {
                      return source_position.min_dist_with(a) > source_position.min_dist_with(b);
                  });
    }

    void route_simple() {
        for (const auto& t : terminal_positions) {
            tree.insert_terminal_node(t);
        }
    }

    void route_with_steiners() {
        for (const auto& t : terminal_positions) {
            tree.insert_terminal_with_steiner(t);
        }
    }

    void route_with_constraints(double alpha = 1.0) {
        int allowed_wirelength = static_cast<int>(std::round(worst_wirelength * alpha));
        for (const auto& t : terminal_positions) {
            tree.insert_terminal_with_constraints(t, allowed_wirelength);
        }
    }

    auto get_tree() const -> const GlobalRoutingTree& { return tree; }
};

extern std::string visualize_routing_tree_svg(const GlobalRoutingTree& tree, int width = 800,
                                              int height = 600, int margin = 50);

extern void save_routing_tree_svg(const GlobalRoutingTree& tree,
                                  std::string filename = "routing_tree.svg", int width = 800,
                                  int height = 600);
