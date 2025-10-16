/**
 * @file global_router.hpp
 * @brief Defines data structures and algorithms for global routing in physical design.
 *
 * This file provides the core components for constructing and manipulating
 * routing trees, including nodes, the tree structure itself, and a global router
 * that can build these trees with various strategies (simple, with Steiner points,
 * and with wirelength constraints). It also includes utilities for visualizing
 * the routing trees.
 */

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

    /// @brief Type alias for a 2D point with integer coordinates.
    using IntPoint = Point<int>;
    /// @brief Type alias for a 2D rectangle defined by two integer intervals.
    using IntRect = Point<Interval<int>, Interval<int>>;

}  // namespace recti

using recti::IntPoint;
using recti::IntRect;

/**
 * @brief Defines the type of a routing node.
 */
enum class NodeType {
    STEINER,  ///< A Steiner point, an intermediate point added to optimize routing.
    TERMINAL, ///< A terminal point, representing a pin or a component connection.
    SOURCE    ///< The source point of the routing tree.
};

/**
 * @brief Converts a NodeType enum value to its string representation.
 * @param t The NodeType to convert.
 * @return A string representation of the NodeType.
 */
inline std::string to_string(const NodeType t) {
    switch (t) {
        case NodeType::STEINER:
            return "Steiner";
        case NodeType::TERMINAL:
            return "Terminal";
        case NodeType::SOURCE:
            return "Source";
    }
    return "Unknown"; // Should not be reached
}

/**
 * @brief Represents a node in the global routing tree.
 *
 * A RoutingNode can be a source, terminal, or Steiner point. It holds
 * its position, type, and connections to parent and child nodes in the tree.
 */
class RoutingNode {
  public:
    std::string id;                ///< Unique identifier for the node.
    NodeType type;                 ///< The type of the node (STEINER, TERMINAL, SOURCE).
    IntPoint pt;                   ///< The 2D integer coordinates of the node.
    std::vector<RoutingNode*> children; ///< Pointers to child nodes in the routing tree.
    RoutingNode* parent = nullptr; ///< Pointer to the parent node. Null for the source node.
    double capacitance = 0.0;      ///< Capacitance associated with the node (for future use).
    double delay = 0.0;            ///< Delay associated with the node (for future use).
    int path_length = 0;           ///< Path length from the source to this node.

    /**
     * @brief Constructs a new RoutingNode.
     * @param id_ The unique identifier for the node.
     * @param type_ The type of the node.
     * @param pt_ The 2D integer coordinates of the node.
     */
    RoutingNode(std::string id_, NodeType type_, IntPoint pt_ = {0, 0})
        : id(std::move(id_)), type(type_), pt(pt_) {}

    /**
     * @brief Adds a child node to this node.
     * @param child A pointer to the child RoutingNode to add.
     */
    void add_child(RoutingNode* child) {
        child->parent = this;
        children.push_back(child);
    }

    /**
     * @brief Removes a child node from this node.
     * @param child A pointer to the child RoutingNode to remove.
     */
    void remove_child(RoutingNode* child) {
        auto it = std::find(children.begin(), children.end(), child);
        if (it != children.end()) {
            children.erase(it);
            child->parent = nullptr;
        }
    }

    /**
     * @brief Gets the position of the routing node.
     * @return A const reference to the IntPoint representing the node's position.
     */
    auto get_position() const -> const IntPoint& { return pt; }

    /**
     * @brief Calculates the Manhattan distance between this node and another node.
     * @param other A pointer to the other RoutingNode.
     * @return The Manhattan distance between the two nodes.
     */
    auto manhattan_distance(const RoutingNode* other) const -> int {
        return pt.min_dist_with(other->pt);
    }
};

/**
 * @brief Represents the entire global routing tree.
 *
 * This class manages the collection of RoutingNodes, including the source,
 * Steiner points, and terminal points, and provides methods for building
 * and manipulating the tree structure.
 */
class GlobalRoutingTree {
    RoutingNode source_node; ///< The root node of the routing tree.
    int next_steiner_id = 1; ///< Counter for generating unique Steiner node IDs.
    int next_terminal_id = 1; ///< Counter for generating unique terminal node IDs.

  public:
    std::unordered_map<std::string, RoutingNode*> nodes; ///< Map from node ID to RoutingNode pointer.
    std::vector<std::unique_ptr<RoutingNode>> owned_nodes; ///< Stores unique_ptrs for memory management of nodes.

  public:
    /**
     * @brief Constructs a new GlobalRoutingTree with a specified source position.
     * @param source_position The 2D integer coordinates of the source node.
     */
    GlobalRoutingTree(IntPoint source_position = {0, 0})
        : source_node("source", NodeType::SOURCE, source_position) {
        nodes["source"] = &source_node;
    }

    /**
     * @brief Gets a const pointer to the source node of the tree.
     * @return A const pointer to the source RoutingNode.
     */
    auto get_source() const -> const RoutingNode* { return &source_node; }

    /**
     * @brief Gets a non-const pointer to the source node of the tree.
     * @return A non-const pointer to the source RoutingNode.
     */
    auto get_source() -> RoutingNode* { return &source_node; }

    /**
     * @brief Inserts a new Steiner node into the routing tree.
     * @param pt The position of the new Steiner node.
     * @param parent_id Optional ID of the parent node. If not provided, the source node is used as parent.
     * @return The ID of the newly inserted Steiner node.
     * @throws std::runtime_error if the specified parent node is not found.
     */
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

    /**
     * @brief Finds the nearest existing node in the tree to a given point.
     * @param pt The target point.
     * @param exclude_id Optional ID of a node to exclude from the search.
     * @return A pointer to the nearest RoutingNode.
     */
    auto _find_nearest_node(IntPoint pt, std::optional<std::string> exclude_id = std::nullopt)
        -> RoutingNode* {
        if (nodes.size() <= 1) return &source_node;
        RoutingNode temp("temp", NodeType::STEINER, pt);
        RoutingNode* nearest = &source_node;
        int min_dist = source_node.manhattan_distance(&temp);
        for (auto& [id, node] : nodes) {
            if (exclude_id && id == *exclude_id) continue;
            int dist = node->manhattan_distance(&temp);
            if (dist < min_dist) {
                min_dist = dist;
                nearest = node;
            }
        }
        return nearest;
    }

    /**
     * @brief Inserts a new terminal node into the routing tree.
     * @param pt The position of the new terminal node.
     * @param parent_id Optional ID of the parent node. If not provided, the nearest existing node is used as parent.
     * @return The ID of the newly inserted terminal node.
     * @throws std::runtime_error if the specified parent node is not found.
     */
    auto insert_terminal_node(IntPoint pt, std::optional<std::string> parent_id = std::nullopt)
        -> std::string {
        std::string terminal_id = "terminal_" + std::to_string(next_terminal_id++);
        auto node_ptr = std::make_unique<RoutingNode>(terminal_id, NodeType::TERMINAL, pt);
        RoutingNode* node = node_ptr.get();
        nodes[terminal_id] = node;
        owned_nodes.push_back(std::move(node_ptr));

        RoutingNode* parent_node;
        if (!parent_id) {
            parent_node = _find_nearest_node(pt, terminal_id);
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

    /**
     * @brief Inserts a new node (Steiner or Terminal) onto an existing branch of the tree.
     * @param new_node_type The type of the new node to insert (STEINER or TERMINAL).
     * @param x The x-coordinate of the new node.
     * @param y The y-coordinate of the new node.
     * @param branch_start_id The ID of the parent node of the branch.
     * @param branch_end_id The ID of the child node of the branch.
     * @return The ID of the newly inserted node.
     * @throws std::runtime_error if branch nodes are not found or if branch_end_id is not a direct child of branch_start_id.
     */
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
    /**
     * @brief Finds the nearest insertion point (node or branch) for a given point.
     * @param pt The target point for insertion.
     * @return An InsertionPair, where the first element is the parent node of the branch (or nullptr if inserting directly to a node),
     *         and the second element is the node to which the new node will be attached (or the child node of the branch).
     */
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

    /**
     * @brief Inserts a terminal node, potentially adding a Steiner node if insertion is on a branch.
     * @param pt The position of the new terminal node.
     */
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

    /**
     * @brief Finds the nearest insertion point (node or branch) for a given point, considering a maximum allowed wirelength.
     * @param pt The target point for insertion.
     * @param allowed_wirelength The maximum allowed wirelength from the source to the potential insertion point.
     * @return An InsertionPair, similar to _find_nearest_insertion, but filtered by wirelength constraint.
     */
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

    /**
     * @brief Inserts a terminal node, potentially adding a Steiner node, with wirelength constraints.
     * @param pt The position of the new terminal node.
     * @param allowed_wirelength The maximum allowed wirelength for the path to the new terminal.
     */
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

    /**
     * @brief Calculates the total wirelength of the routing tree.
     * @return The total wirelength as an integer.
     */
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

    /**
     * @brief Generates a string representation of the tree structure for visualization or debugging.
     * @param node The starting node for the traversal (defaults to source_node).
     * @param level The current depth level in the tree (for indentation).
     * @return A string representing the tree structure.
     */
    auto get_tree_structure(const RoutingNode* node = nullptr, int level = 0) const -> std::string;

    /**
     * @brief Finds the path from a given node to the source node.
     * @param node_id The ID of the starting node.
     * @return A vector of const pointers to RoutingNodes, representing the path from source to the given node.
     * @throws std::runtime_error if the specified node is not found.
     */
    auto find_path_to_source(const std::string& node_id) const -> std::vector<const RoutingNode*> {
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

    /**
     * @brief Retrieves all terminal nodes in the tree.
     * @return A vector of pointers to all RoutingNodes of type TERMINAL.
     */
    auto get_all_terminals() const -> std::vector<RoutingNode*> {
        std::vector<RoutingNode*> terms;
        for (auto& [id, node] : nodes) {
            if (node->type == NodeType::TERMINAL) {
                terms.push_back(node);
            }
        }
        return terms;
    }

    /**
     * @brief Retrieves all Steiner nodes in the tree.
     * @return A vector of pointers to all RoutingNodes of type STEINER.
     */
    auto get_all_steiner_nodes() const -> std::vector<RoutingNode*> {
        std::vector<RoutingNode*> steins;
        for (auto& [id, node] : nodes) {
            if (node->type == NodeType::STEINER) {
                steins.push_back(node);
            }
        }
        return steins;
    }

    /**
     * @brief Optimizes the routing tree by removing redundant Steiner points.
     * A Steiner point is considered redundant if it has only one child and is not the source node.
     */
    void optimize_steiner_points() {
        std::vector<std::string> steiner_ids_to_remove;
        for (auto& [id, node] : nodes) {
            if (node->type == NodeType::STEINER && node->children.size() == 1
                && node->parent != nullptr && node->parent->type != NodeType::SOURCE) {
                steiner_ids_to_remove.push_back(id);
            }
        }

        for (const auto& steiner_id : steiner_ids_to_remove) {
            RoutingNode* steiner = nodes.at(steiner_id);
            RoutingNode* parent = steiner->parent;
            RoutingNode* child = steiner->children[0];

            parent->remove_child(steiner);
            parent->add_child(child);
            child->parent = parent;  // Update child's parent

            nodes.erase(steiner_id);
            auto owned_it = std::find_if(owned_nodes.begin(), owned_nodes.end(),
                                         [steiner](const auto& up) { return up.get() == steiner; });
            if (owned_it != owned_nodes.end()) {
                owned_nodes.erase(owned_it);
            }
        }
    }

    /**
     * @brief Visualizes the routing tree (implementation in .cpp file).
     */
    void visualize_tree() const;
};

/**
 * @brief Manages the global routing process, constructing a GlobalRoutingTree.
 *
 * This class takes a source position and a list of terminal positions
 * and can build a routing tree using different strategies.
 */
class GlobalRouter {
    IntPoint source_position; ///< The starting position for the routing.
    std::vector<IntPoint> terminal_positions; ///< The target terminal positions.
    GlobalRoutingTree tree; ///< The routing tree constructed by this router.
    int worst_wirelength = 0; ///< The maximum wirelength from source to any terminal, used for constraints.

  public:
    /**
     * @brief Constructs a new GlobalRouter.
     * @param source_ The source position.
     * @param terminals A vector of terminal positions.
     */
    GlobalRouter(const IntPoint& source_, const std::vector<IntPoint>& terminals)
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

    /**
     * @brief Routes terminals simply by connecting each to the nearest existing node.
     */
    void route_simple() {
        for (const auto& t : terminal_positions) {
            tree.insert_terminal_node(t);
        }
    }

    /**
     * @brief Routes terminals, potentially inserting Steiner nodes to optimize connections.
     */
    void route_with_steiners() {
        for (const auto& t : terminal_positions) {
            tree.insert_terminal_with_steiner(t);
        }
    }

    /**
     * @brief Routes terminals with wirelength constraints.
     * @param alpha A multiplier for the worst wirelength to determine the allowed wirelength.
     */
    void route_with_constraints(double alpha = 1.0) {
        int allowed_wirelength = static_cast<int>(std::round(worst_wirelength * alpha));
        for (const auto& t : terminal_positions) {
            tree.insert_terminal_with_constraints(t, allowed_wirelength);
        }
    }

    /**
     * @brief Gets a const reference to the constructed GlobalRoutingTree.
     * @return A const reference to the GlobalRoutingTree.
     */
    auto get_tree() const -> const GlobalRoutingTree& { return tree; }
};

/**
 * @brief Generates an SVG string representation of the routing tree.
 * @param tree The GlobalRoutingTree to visualize.
 * @param width The width of the SVG image.
 * @param height The height of the SVG image.
 * @param margin The margin around the routing tree within the SVG.
 * @return A string containing the SVG representation.
 */
extern std::string visualize_routing_tree_svg(const GlobalRoutingTree& tree, const int width = 800,
                                              const int height = 600, const int margin = 50);

/**
 * @brief Saves an SVG representation of the routing tree to a file.
 * @param tree The GlobalRoutingTree to save.
 * @param filename The name of the output SVG file.
 * @param width The width of the SVG image.
 * @param height The height of the SVG image.
 */
extern void save_routing_tree_svg(const GlobalRoutingTree& tree,
                                  const std::string filename = "routing_tree.svg",
                                  const int width = 800, const int height = 600);
