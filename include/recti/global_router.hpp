// global_router.hpp
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
#include <unordered_map>
#include <vector>

/**
 * @brief Defines the type of a routing node.
 */
enum class NodeType {
    STEINER,   ///< A Steiner point, an intermediate point added to optimize routing.
    TERMINAL,  ///< A terminal point, representing a pin or a component connection.
    SOURCE     ///< The source point of the routing tree.
};

/**
 * @brief Converts a NodeType enum value to its string representation.
 * @param node_type The NodeType to convert.
 * @return A string representation of the NodeType.
 */
inline std::string to_string(const NodeType node_type) {
    switch (node_type) {
        case NodeType::STEINER:
            return "Steiner";
        case NodeType::TERMINAL:
            return "Terminal";
        case NodeType::SOURCE:
            return "Source";
    }
    return "Unknown";  // Should not be reached
}

/**
 * @brief Represents a node in the global routing tree.
 *
 * A RoutingNode<IntPoint> can be a source, terminal, or Steiner point. It holds
 * its position, type, and connections to parent and child nodes in the tree.
 */
template <typename IntPoint> class RoutingNode {
  public:
    std::string id;  ///< Unique identifier for the node.
    NodeType type;   ///< The type of the node (STEINER, TERMINAL, SOURCE).
    IntPoint pt;     ///< The 2D integer coordinates of the node.
    std::vector<RoutingNode<IntPoint>*> children;  ///< Pointers to child nodes in the routing tree.
    RoutingNode<IntPoint>* parent
        = nullptr;             ///< Pointer to the parent node. Null for the source node.
    double capacitance = 0.0;  ///< Capacitance associated with the node (for future use).
    double delay = 0.0;        ///< Delay associated with the node (for future use).
    int path_length = 0;       ///< Path length from the source to this node.

    /**
     * @brief Constructs a new RoutingNode<IntPoint>.
     * @param id_ The unique identifier for the node.
     * @param type_ The type of the node.
     * @param pt_ The 2D integer coordinates of the node.
     */
    RoutingNode(std::string id_, NodeType type_, IntPoint pt_)
        : id(std::move(id_)), type(type_), pt(pt_) {}

    /**
     * @brief Adds a child node to this node.
     * @param child A pointer to the child RoutingNode<IntPoint> to add.
     */
    void add_child(RoutingNode<IntPoint>* child) {
        child->parent = this;
        children.push_back(child);
    }

    /**
     * @brief Removes a child node from this node.
     * @param child A pointer to the child RoutingNode<IntPoint> to remove.
     */
    void remove_child(RoutingNode<IntPoint>* child) {
        auto iter = std::find(children.begin(), children.end(), child);
        if (iter != children.end()) {
            children.erase(iter);
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
     * @param other A pointer to the other RoutingNode<IntPoint>.
     * @return The Manhattan distance between the two nodes.
     */
    auto manhattan_distance(const RoutingNode<IntPoint>* other) const -> int {
        return pt.min_dist_with(other->pt);
    }
};

/**
 * @brief Represents the entire global routing tree.
 *
 * This class manages the collection of RoutingNodes, including the source,
 * Steiner points, and terminal points, and provides methods for building
 * and manipulating the tree structure.
 *
 * @code{.txt}
 * +-----------------------------+
 * |      GlobalRoutingTree      |
 * +-----------------------------+
 * | source_node (SOURCE)        |
 * |    |                        |
 * |    v                        |
 * | +-------------------------+  |
 * | | RoutingNode             |  |
 * | | id: source              |  |
 * | | type: SOURCE            |  |  A tree structure with
 * | | pt: (x, y)              |  |  source, terminals,
 * | | children[]              |  |  and steiner points
 * | | parent: null            |  |
 * | +-------------------------+  |
 * |    |                        |
 * |    +--> +----------------+   |
 * |         |RoutingNode     |   |
 * |         |id: terminal_1  |   |
 * |         |type: TERMINAL  |   |
 * |         |pt: (x1, y1)    |   |
 * |         |children[]      |   |
 * |         |parent: source  |   |
 * |         +----------------+   |
 * |    |                        |
 * |    +--> +----------------+   |
 * |         |RoutingNode     |   |
 * |         |id: steiner_1   |   |
 * |         |type: STEINER   |   |
 * |         |pt: (x2, y2)    |   |
 * |         |children[]      |   |
 * |         |parent: source  |   |
 * |         +----------------+   |
 * |            |                 |
 * |            +--> +----------+ |
 * |                 |RoutingNode| |
 * |                 |id: term_2| |
 * |                 |type: TERM| |
 * |                 |pt: (x3,y3)||
 * |                 |children[]| |
 * |                 |parent: st| |
 * |                 +----------+ |
 * +-----------------------------+
 * @endcode
 */
template <typename IntPoint> class GlobalRoutingTree {
  public:
    using Keepout = decltype(std::declval<IntPoint>().enlarge_with(1));

  private:
    RoutingNode<IntPoint> source_node;  ///< The root node of the routing tree.
    int next_steiner_id = 1;            ///< Counter for generating unique Steiner node IDs.
    int next_terminal_id = 1;           ///< Counter for generating unique terminal node IDs.

    auto _find_nearest_node(const IntPoint& pt, std::optional<std::string> exclude_id
                                                = std::nullopt) -> RoutingNode<IntPoint>* {
        if (this->nodes.size() <= 1) return &this->source_node;
        RoutingNode<IntPoint>* nearest = &this->source_node;
        int min_dist = this->source_node.pt.min_dist_with(pt);
        for (auto& [id, node] : this->nodes) {
            if (exclude_id && id == *exclude_id) continue;
            int dist = node->pt.min_dist_with(pt);
            if (dist < min_dist) {
                min_dist = dist;
                nearest = node;
            }
        }
        return nearest;
    }

    auto _find_nearest_insertion(const IntPoint& pt,
                                 std::optional<std::vector<Keepout>> keepouts = std::nullopt)
        -> std::pair<RoutingNode<IntPoint>*, RoutingNode<IntPoint>*> {
        return _find_nearest_insertion_with_constraints(pt, std::nullopt, keepouts);
    }

    auto _find_nearest_insertion_with_constraints(
        const IntPoint& pt, std::optional<int> allowed_wirelength,
        std::optional<std::vector<Keepout>> keepouts = std::nullopt)
        -> std::pair<RoutingNode<IntPoint>*, RoutingNode<IntPoint>*> {
        RoutingNode<IntPoint>* parent_node = nullptr;
        RoutingNode<IntPoint>* nearest_node = &this->source_node;
        int min_distance = this->source_node.pt.min_dist_with(pt);

        std::function<void(RoutingNode<IntPoint>*)> traverse = [&](RoutingNode<IntPoint>* node) {
            for (auto* child : node->children) {
                auto possible_path = node->pt.hull_with(child->pt);
                int distance = possible_path.min_dist_with(pt);
                auto nearest_pt = possible_path.nearest_to(pt);
                if (allowed_wirelength) {
                    int path_length
                        = node->path_length + node->pt.min_dist_with(nearest_pt) + distance;
                    if (path_length > *allowed_wirelength) continue;
                }
                if (distance < min_distance) {
                    bool block = false;
                    if (keepouts.has_value()) {
                        auto path1 = nearest_pt.hull_with(pt);
                        auto path2 = nearest_pt.hull_with(node->pt);
                        auto path3 = nearest_pt.hull_with(child->pt);
                        for (const auto& keepout : *keepouts) {
                            if (keepout.contains(nearest_pt)) {
                                block = true;
                            }
                            if (keepout.blocks(path1) || keepout.blocks(path2)
                                || keepout.blocks(path3)) {
                                block = true;
                            }
                        }
                    }
                    if (!block) {
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
                }
                traverse(child);
            }
        };
        traverse(&this->source_node);
        return {parent_node, nearest_node};
    }

    auto _insert_terminal_impl(const IntPoint& pt, std::optional<int> allowed_wirelength,
                               std::optional<std::vector<Keepout>> keepouts) -> void {
        std::string terminal_id = "terminal_" + std::to_string(this->next_terminal_id++);
        auto terminal_ptr
            = std::make_unique<RoutingNode<IntPoint>>(terminal_id, NodeType::TERMINAL, pt);
        RoutingNode<IntPoint>* terminal_node = terminal_ptr.get();
        this->nodes[terminal_id] = terminal_node;
        this->owned_nodes.push_back(std::move(terminal_ptr));

        auto [parent_node, nearest_node]
            = this->_find_nearest_insertion_with_constraints(pt, allowed_wirelength, keepouts);

        if (parent_node == nullptr) {
            nearest_node->add_child(terminal_node);
            terminal_node->path_length
                = nearest_node->path_length + nearest_node->pt.min_dist_with(pt);
        } else {
            std::string steiner_id = "steiner_" + std::to_string(this->next_steiner_id++);
            auto possible_path = parent_node->pt.hull_with(nearest_node->pt);
            IntPoint nearest_pt = possible_path.nearest_to(pt);
            auto steiner_ptr = std::make_unique<RoutingNode<IntPoint>>(
                steiner_id, NodeType::STEINER, nearest_pt);
            RoutingNode<IntPoint>* new_node = steiner_ptr.get();
            this->nodes[steiner_id] = new_node;
            this->owned_nodes.push_back(std::move(steiner_ptr));

            parent_node->remove_child(nearest_node);
            parent_node->add_child(new_node);
            new_node->path_length
                = parent_node->path_length + parent_node->pt.min_dist_with(nearest_pt);
            new_node->add_child(nearest_node);
            new_node->add_child(terminal_node);
            terminal_node->path_length = new_node->path_length + nearest_pt.min_dist_with(pt);
        }
    }

  public:
    std::unordered_map<std::string, RoutingNode<IntPoint>*>
        nodes;  ///< Map from node ID to RoutingNode<IntPoint> pointer.
    std::vector<std::unique_ptr<RoutingNode<IntPoint>>>
        owned_nodes;  ///< Stores unique_ptrs for memory management of nodes.

    /**
     * @brief Constructs a new GlobalRoutingTree with a specified source position.
     * @param source_position The 2D integer coordinates of the source node.
     */
    GlobalRoutingTree(IntPoint source_position)
        : source_node("source", NodeType::SOURCE, source_position) {
        nodes["source"] = &source_node;
    }

    /**
     * @brief Gets a const pointer to the source node of the tree.
     * @return A const pointer to the source RoutingNode<IntPoint>.
     */
    auto get_source() const -> const RoutingNode<IntPoint>* { return &source_node; }

    /**
     * @brief Gets a non-const pointer to the source node of the tree.
     * @return A non-const pointer to the source RoutingNode<IntPoint>.
     */
    auto get_source() -> RoutingNode<IntPoint>* { return &source_node; }

    /**
     * @brief Inserts a new Steiner node into the routing tree.
     * @param pt The position of the new Steiner node.
     * @param parent_id Optional ID of the parent node. If not provided, the source node is used as
     * parent.
     * @return The ID of the newly inserted Steiner node.
     * @throws std::runtime_error if the specified parent node is not found.
     */
    auto insert_steiner_node(const IntPoint& pt,
                             std::optional<std::string> parent_id = std::nullopt) -> std::string {
        std::string steiner_id = "steiner_" + std::to_string(this->next_steiner_id++);
        auto node_ptr = std::make_unique<RoutingNode<IntPoint>>(steiner_id, NodeType::STEINER, pt);
        RoutingNode<IntPoint>* node = node_ptr.get();
        this->nodes[steiner_id] = node;
        this->owned_nodes.push_back(std::move(node_ptr));

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

    /**
     * @brief Inserts a new terminal node into the routing tree.
     * @param pt The position of the new terminal node.
     * @param parent_id Optional ID of the parent node. If not provided, the nearest existing node
     * is used as parent.
     * @return The ID of the newly inserted terminal node.
     * @throws std::runtime_error if the specified parent node is not found.
     */
    auto insert_terminal_node(const IntPoint& pt,
                              std::optional<std::string> parent_id = std::nullopt) -> std::string {
        std::string terminal_id = "terminal_" + std::to_string(this->next_terminal_id++);
        auto node_ptr
            = std::make_unique<RoutingNode<IntPoint>>(terminal_id, NodeType::TERMINAL, pt);
        RoutingNode<IntPoint>* node = node_ptr.get();
        this->nodes[terminal_id] = node;
        this->owned_nodes.push_back(std::move(node_ptr));

        RoutingNode<IntPoint>* parent_node;
        if (!parent_id) {
            parent_node = this->_find_nearest_node(pt);
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

    /**
     * @brief Inserts a new node (Steiner or Terminal) onto an existing branch of the tree.
     * @param new_node_type The type of the new node to insert (STEINER or TERMINAL).
     * @param pt The position of the new node.
     * @param branch_start_id The ID of the parent node of the branch.
     * @param branch_end_id The ID of the child node of the branch.
     * @return The ID of the newly inserted node.
     * @throws std::runtime_error if branch nodes are not found or if branch_end_id is not a direct
     * child of branch_start_id.
     */
    auto insert_node_on_branch(NodeType new_node_type, const IntPoint& pt,
                               std::string branch_start_id, std::string branch_end_id)
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
        auto node_ptr = std::make_unique<RoutingNode<IntPoint>>(node_id, new_node_type, pt);
        RoutingNode<IntPoint>* new_node = node_ptr.get();
        this->nodes[node_id] = new_node;
        this->owned_nodes.push_back(std::move(node_ptr));

        start_node->remove_child(end_node);
        start_node->add_child(new_node);
        new_node->add_child(end_node);
        return node_id;
    }

    /**
     * @brief Inserts a new terminal node with possible Steiner point.
     * @param pt The position of the terminal.
     * @param keepouts Optional keepouts to avoid.
     */
    auto insert_terminal_with_steiner(const IntPoint& pt,
                                      std::optional<std::vector<Keepout>> keepouts = std::nullopt)
        -> void {
        _insert_terminal_impl(pt, std::nullopt, keepouts);
    }

    /**
     * @brief Inserts a new terminal node with constraints.
     * @param pt The position of the terminal.
     * @param allowed_wirelength The allowed wirelength.
     * @param keepouts Optional keepouts to avoid.
     */
    auto insert_terminal_with_constraints(const IntPoint& pt, int allowed_wirelength,
                                          std::optional<std::vector<Keepout>> keepouts
                                          = std::nullopt) -> void {
        _insert_terminal_impl(pt, allowed_wirelength, keepouts);
    }

    /**
     * @brief Calculates the total wirelength of the routing tree.
     * @return The total wirelength as an integer.
     */
    auto calculate_wirelength() const -> int {
        int total = 0;
        std::function<void(const RoutingNode<IntPoint>*)> traverse
            = [&](const RoutingNode<IntPoint>* node) {
                  for (auto child : node->children) {
                      total += node->manhattan_distance(child);
                      traverse(child);
                  }
              };
        traverse(&source_node);
        return total;
    }

    /**
     * @brief Generates a string representation of the tree structure for visualization or
     * debugging.
     * @param node The starting node for the traversal (defaults to source_node).
     * @param level The current depth level in the tree (for indentation).
     * @return A string representing the tree structure.
     */
    auto get_tree_structure(const RoutingNode<IntPoint>* node = nullptr, int level = 0) const
        -> std::string;

    /**
     * @brief Finds the path from a given node to the source node.
     * @param node_id The ID of the starting node.
     * @return A vector of const pointers to RoutingNodes, representing the path from source to the
     * given node.
     * @throws std::runtime_error if the specified node is not found.
     */
    auto find_path_to_source(const std::string& node_id) const
        -> std::vector<const RoutingNode<IntPoint>*> {
        auto iter = nodes.find(node_id);
        if (iter == nodes.end()) {
            throw std::runtime_error("Node " + node_id + " not found");
        }
        const RoutingNode<IntPoint>* current = iter->second;
        std::vector<const RoutingNode<IntPoint>*> path;
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
    auto get_all_terminals() const -> std::vector<const RoutingNode<IntPoint>*> {
        std::vector<const RoutingNode<IntPoint>*> terms;
        for (auto& pair : this->nodes) {
            const auto& node = pair.second;
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
    auto get_all_steiner_nodes() const -> std::vector<const RoutingNode<IntPoint>*> {
        std::vector<const RoutingNode<IntPoint>*> steins;
        for (auto& pair : this->nodes) {
            const auto& node = pair.second;
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
        for (auto& [id, node] : this->nodes) {
            if (node->type == NodeType::STEINER && node->children.size() == 1
                && node->parent != nullptr) {
                steiner_ids_to_remove.push_back(id);
            }
        }

        for (const auto& steiner_id : steiner_ids_to_remove) {
            RoutingNode<IntPoint>* steiner = this->nodes.at(steiner_id);
            RoutingNode<IntPoint>* parent = steiner->parent;
            RoutingNode<IntPoint>* child = steiner->children[0];

            parent->remove_child(steiner);
            parent->add_child(child);

            this->nodes.erase(steiner_id);
            auto owned_iter = std::find_if(this->owned_nodes.begin(), this->owned_nodes.end(),
                                         [steiner](const auto& up) { return up.get() == steiner; });
            if (owned_iter != this->owned_nodes.end()) {
                this->owned_nodes.erase(owned_iter);
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
template <typename IntPoint> class GlobalRouter {
    IntPoint source_position;                  ///< The starting position for the routing.
    std::vector<IntPoint> terminal_positions;  ///< The target terminal positions.
    GlobalRoutingTree<IntPoint> tree;          ///< The routing tree constructed by this router.
    int worst_wirelength
        = 0;  ///< The maximum wirelength from source to any terminal, used for constraints.
    std::optional<std::vector<typename GlobalRoutingTree<IntPoint>::Keepout>>
        keepouts;  ///< Optional keepouts.

  public:
    using Keepout = decltype(std::declval<IntPoint>().enlarge_with(1));

    /**
     * @brief Constructs a new GlobalRouter.
     * @param source_ The source position.
     * @param terminal_positions A vector of terminal positions.
     * @param keepouts_ Optional keepouts.
     */
    GlobalRouter(const IntPoint& source_, std::vector<IntPoint> terminal_positions,
                 std::optional<std::vector<Keepout>> keepouts_ = std::nullopt)
        : source_position(source_), tree(source_), keepouts(keepouts_) {
        // Sort terminal positions by distance from source (descending order)
        std::sort(terminal_positions.begin(), terminal_positions.end(),
                  [this](const IntPoint& pt_a, const IntPoint& pt_b) {
                      auto dist_a = this->source_position.min_dist_with(pt_a);
                      auto dist_b = this->source_position.min_dist_with(pt_b);
                      return dist_a < dist_b
                             || (dist_a == dist_b
                                 && this->source_position.hull_with(pt_a).measure()
                                        > this->source_position.hull_with(pt_b).measure());
                  });
        this->terminal_positions = std::move(terminal_positions);

        // Calculate worst wirelength (distance to farthest terminal)
        if (!this->terminal_positions.empty()) {
            this->worst_wirelength = this->source_position.min_dist_with(this->terminal_positions.back());
        } else {
            this->worst_wirelength = 0;
        }
    }

    /**
     * @brief Routes terminals simply by connecting each to the nearest existing node.
     */
    void route_simple() {
        for (const auto& terminal : this->terminal_positions) {
            this->tree.insert_terminal_node(terminal);
        }
    }

    /**
     * @brief Routes terminals, potentially inserting Steiner nodes to optimize connections.
     */
    void route_with_steiners() {
        for (const auto& terminal : this->terminal_positions) {
            this->tree.insert_terminal_with_steiner(terminal, this->keepouts);
        }
    }

    /**
     * @brief Routes terminals with wirelength constraints.
     * @param alpha A multiplier for the worst wirelength to determine the allowed wirelength.
     */
    void route_with_constraints(double alpha = 1.0) {
        int allowed_wirelength = static_cast<int>(std::round(this->worst_wirelength * alpha));
        for (const auto& terminal : this->terminal_positions) {
            this->tree.insert_terminal_with_constraints(terminal, allowed_wirelength, this->keepouts);
        }
    }

    /**
     * @brief Gets a const reference to the constructed GlobalRoutingTree<IntPoint>.
     * @return A const reference to the GlobalRoutingTree<IntPoint>.
     */
    auto get_tree() const -> const GlobalRoutingTree<IntPoint>& { return this->tree; }
};

template <typename IntPoint> extern std::string visualize_routing_tree_svg(
    const GlobalRoutingTree<IntPoint>& tree,
    std::optional<std::vector<typename GlobalRoutingTree<IntPoint>::Keepout>> keepouts
    = std::nullopt,
    const int width = 800, const int height = 600, const int margin = 50);

template <typename IntPoint> extern void save_routing_tree_svg(
    const GlobalRoutingTree<IntPoint>& tree,
    std::optional<std::vector<typename GlobalRoutingTree<IntPoint>::Keepout>> keepouts
    = std::nullopt,
    const std::string filename = "routing_tree.svg", const int width = 800, const int height = 600);

template <typename IntPoint> extern std::string visualize_routing_tree3d_svg(
    const GlobalRoutingTree<IntPoint>& tree,
    std::optional<std::vector<typename GlobalRoutingTree<IntPoint>::Keepout>> keepouts
    = std::nullopt,
    const int scale_z = 100, const int width = 800, const int height = 600, const int margin = 50);

template <typename IntPoint> extern void save_routing_tree3d_svg(
    const GlobalRoutingTree<IntPoint>& tree,
    std::optional<std::vector<typename GlobalRoutingTree<IntPoint>::Keepout>> keepouts
    = std::nullopt,
    const int scale_z = 100, const std::string filename = "routing_tree3d.svg",
    const int width = 800, const int height = 600);