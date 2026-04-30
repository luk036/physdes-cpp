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
#include <memory>
#include <optional>
#include <recti/generic.hpp>  // for enlarge_with
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace recti {

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
     * @param routing_node_type The NodeType to convert.
     * @return A string representation of the NodeType.
     */
    std::string to_string(const NodeType routing_node_type);

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
        std::vector<RoutingNode<IntPoint>*>
            children;  ///< Pointers to child nodes in the routing tree.
        RoutingNode<IntPoint>* parent
            = nullptr;             ///< Pointer to the parent node. Null for the source node.
        double capacitance = 0.0;  ///< Capacitance associated with the node (for future use).
        double delay = 0.0;        ///< Delay associated with the node (for future use).
        int path_length = 0;       ///< Path length from the source to this node.

        /**
         * @brief Constructs a new RoutingNode<IntPoint>.
         * @param node_id The unique identifier for the node.
         * @param node_type The type of the node.
         * @param node_position The 2D integer coordinates of the node.
         */
        RoutingNode(std::string node_id, NodeType node_type, IntPoint node_position)
            : id(std::move(node_id)), type(node_type), pt(node_position) {}

        /**
         * @brief Adds a child node to this node.
         * @param child_node A pointer to the child RoutingNode<IntPoint> to add.
         */
        void add_child(RoutingNode<IntPoint>* child_node) {
            child_node->parent = this;
            children.emplace_back(child_node);
        }

        /**
         * @brief Removes a child node from this node.
         * @param child_node A pointer to the child RoutingNode<IntPoint> to remove.
         */
        void remove_child(RoutingNode<IntPoint>* child_node) {
            auto iter = std::find(children.begin(), children.end(), child_node);
            if (iter != children.end()) {
                children.erase(iter);
                child_node->parent = nullptr;
            }
        }

        /**
         * @brief Gets the position of the routing node.
         * @return A const reference to the IntPoint representing the node's position.
         */
        auto get_position() const -> const IntPoint& { return pt; }

        /**
         * @brief Calculates the Manhattan distance between this node and another node.
         * @param other_node A pointer to the other RoutingNode<IntPoint>.
         * @return The Manhattan distance between the two nodes.
         */
        auto manhattan_distance(const RoutingNode<IntPoint>* other_node) const -> int {
            return pt.min_dist_with(other_node->pt);
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

        auto _find_nearest_node(const IntPoint& point, std::optional<std::string> exclude_id
                                                       = std::nullopt) -> RoutingNode<IntPoint>*;

        // auto _find_nearest_insertion_with_constraints_old(const IntPoint& pt,
        //                                               int allowed_wirelength =
        //                                               std::numeric_limits<int>::max(),
        //                                               std::optional<std::vector<Keepout>>
        //                                               keepouts = std::nullopt)
        //     -> std::pair<RoutingNode<IntPoint>*, RoutingNode<IntPoint>*> {
        //     RoutingNode<IntPoint>* parent_node = nullptr;
        //     RoutingNode<IntPoint>* nearest_node = &this->source_node;
        //     int min_distance = this->source_node.pt.min_dist_with(pt);

        //     std::function<void(RoutingNode<IntPoint>*)> traverse = [&](RoutingNode<IntPoint>*
        //     node) {
        //         for (auto* child : node->children) {
        //             auto possible_path = node->pt.hull_with(child->pt);
        //             int distance = possible_path.min_dist_with(pt);
        //             auto nearest_pt = possible_path.nearest_to(pt);
        //             int path_length
        //                 = node->path_length + node->pt.min_dist_with(nearest_pt) + distance;
        //             if (path_length > allowed_wirelength) continue;
        //             if (distance < min_distance) {
        //                 bool block = false;
        //                 if (keepouts.has_value()) {
        //                     auto path1 = nearest_pt.hull_with(pt);
        //                     auto path2 = nearest_pt.hull_with(node->pt);
        //                     auto path3 = nearest_pt.hull_with(child->pt);
        //                     for (const auto& keepout : *keepouts) {
        //                         if (keepout.contains(nearest_pt)) {
        //                             block = true;
        //                             break;
        //                         }
        //                         if (keepout.blocks(path1) || keepout.blocks(path2)
        //                             || keepout.blocks(path3)) {
        //                             block = true;
        //                             break;
        //                         }
        //                     }
        //                 }
        //                 if (!block) {
        //                     min_distance = distance;
        //                     if (nearest_pt == node->pt) {
        //                         nearest_node = node;
        //                         parent_node = nullptr;
        //                     } else if (nearest_pt == child->pt) {
        //                         nearest_node = child;
        //                         parent_node = nullptr;
        //                     } else {
        //                         parent_node = node;
        //                         nearest_node = child;
        //                     }
        //                 }
        //             }
        //             traverse(child);
        //         }
        //     };
        //     traverse(&this->source_node);
        //     return {parent_node, nearest_node};
        // }

        auto _find_nearest_insertion_with_constraints(const IntPoint& pt,
                                                      int allowed_wirelength
                                                      = std::numeric_limits<int>::max(),
                                                      std::optional<std::vector<Keepout>> keepouts
                                                      = std::nullopt)
            -> std::pair<RoutingNode<IntPoint>*, RoutingNode<IntPoint>*>;

        auto _insert_terminal_impl(const IntPoint& point,
                                   int allowed_wirelength = std::numeric_limits<int>::max(),
                                   std::optional<std::vector<Keepout>> keepouts = std::nullopt)
            -> void;

      public:
        std::unordered_map<std::string, RoutingNode<IntPoint>*>
            nodes;  ///< Map from node ID to RoutingNode<IntPoint> pointer.
        std::vector<std::unique_ptr<RoutingNode<IntPoint>>>
            owned_nodes;           ///< Stores unique_ptrs for memory management of nodes.
        int worst_wirelength = 0;  ///< The worst-case wirelength constraint for routing (used in
                                   ///< constrained routing).

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
         * @param point The position of the new Steiner node.
         * @param parent_id Optional ID of the parent node. If not provided, the source node is used
         * as parent.
         * @return The ID of the newly inserted Steiner node.
         * @throws std::runtime_error if the specified parent node is not found.
         */
        auto insert_steiner_node(const IntPoint& point, std::optional<std::string> parent_id
                                                        = std::nullopt) -> std::string;

        /**
         * @brief Inserts a new terminal node into the routing tree.
         * @param point The position of the new terminal node.
         * @param parent_id Optional ID of the parent node. If not provided, the nearest existing
         * node is used as parent.
         * @return The ID of the newly inserted terminal node.
         * @throws std::runtime_error if the specified parent node is not found.
         */
        auto insert_terminal_node(const IntPoint& point, std::optional<std::string> parent_id
                                                         = std::nullopt) -> std::string;

        /**
         * @brief Inserts a new node (Steiner or Terminal) onto an existing branch of the tree.
         * @param new_node_type The type of the new node to insert (STEINER or TERMINAL).
         * @param point The position of the new node.
         * @param branch_start_id The ID of the parent node of the branch.
         * @param branch_end_id The ID of the child node of the branch.
         * @return The ID of the newly inserted node.
         * @throws std::runtime_error if branch nodes are not found or if branch_end_id is not a
         * direct child of branch_start_id.
         */
        auto insert_node_on_branch(NodeType new_node_type, const IntPoint& point,
                                   std::string branch_start_id, std::string branch_end_id)
            -> std::string;

        /**
         * @brief Inserts a new terminal node with possible Steiner point.
         * @param point The position of the terminal.
         * @param keepouts Optional keepouts to avoid.
         */
        auto insert_terminal_with_steiner(const IntPoint& point,
                                          std::optional<std::vector<Keepout>> keepouts
                                          = std::nullopt) -> void {
            _insert_terminal_impl(point, std::numeric_limits<int>::max(), keepouts);
        }

        /**
         * @brief Inserts a new terminal node with constraints.
         * @param point The position of the terminal.
         * @param allowed_wirelength The allowed wirelength.
         * @param keepouts Optional keepouts to avoid.
         */
        auto insert_terminal_with_constraints(const IntPoint& point, int allowed_wirelength,
                                              std::optional<std::vector<Keepout>> keepouts
                                              = std::nullopt) -> void {
            _insert_terminal_impl(point, allowed_wirelength, keepouts);
        }

        /**
         * @brief Calculates the total wirelength of the routing tree.
         * @return The total wirelength as an integer.
         */
        auto calculate_total_wirelength() const -> int;

        /**
         * @brief Calculates the worst wirelength of the routing tree.
         * @return The worst wirelength as an integer.
         */
        auto calculate_worst_wirelength() const -> int;

        /**
         * @brief Generates a string representation of the tree structure for visualization or
         * debugging.
         * @param current_node The starting node for the traversal (defaults to source_node).
         * @param level The current depth level in the tree (for indentation).
         * @return A string representing the tree structure.
         */
        auto get_tree_structure(const RoutingNode<IntPoint>* current_node = nullptr,
                                int level = 0) const -> std::string;

        /**
         * @brief Finds the path from a given node to the source node.
         * @param node_id The ID of the starting node.
         * @return A vector of const pointers to RoutingNodes, representing the path from source to
         * the given node.
         * @throws std::runtime_error if the specified node is not found.
         */
        auto find_path_to_source(const std::string& node_id) const
            -> std::vector<const RoutingNode<IntPoint>*>;

        /**
         * @brief Retrieves all terminal nodes in the tree.
         * @return A vector of pointers to all RoutingNodes of type TERMINAL.
         */
        auto get_all_terminals() const -> std::vector<const RoutingNode<IntPoint>*>;

        /**
         * @brief Retrieves all Steiner nodes in the tree.
         * @return A vector of pointers to all RoutingNodes of type STEINER.
         */
        auto get_all_steiner_nodes() const -> std::vector<const RoutingNode<IntPoint>*>;

        /**
         * @brief Optimizes the routing tree by removing redundant Steiner points.
         * A Steiner point is considered redundant if it has only one child and is not the source
         * node.
         */
        void optimize_steiner_points();

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
         * @param source_pos The source position.
         * @param terminal_positions A vector of terminal positions.
         * @param keepout_regions Optional keepouts.
         */
        GlobalRouter(const IntPoint& source_pos, std::vector<IntPoint> terminal_positions,
                     std::optional<std::vector<Keepout>> keepout_regions = std::nullopt);

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
            this->tree.worst_wirelength = this->worst_wirelength;  // Store the allowed wirelength
                                                                   // in the tree for reference.q
            for (const auto& terminal : this->terminal_positions) {
                this->tree.insert_terminal_with_steiner(terminal, this->keepouts);
            }
        }

        /**
         * @brief Routes terminals with wirelength constraints.
         * @param multiplier A multiplier for the worst wirelength to determine the allowed
         * wirelength.
         */
        void route_with_constraints(double multiplier = 1.0) {
            int allowed_wirelength
                = static_cast<int>(std::round(this->worst_wirelength * multiplier));
            this->tree.worst_wirelength = this->worst_wirelength;  // Store the allowed wirelength
                                                                   // in the tree for reference.q
            for (const auto& terminal : this->terminal_positions) {
                this->tree.insert_terminal_with_constraints(terminal, allowed_wirelength,
                                                            this->keepouts);
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
        const std::string filename = "routing_tree.svg", const int width = 800,
        const int height = 600);

    template <typename IntPoint> extern std::string visualize_routing_tree3d_svg(
        const GlobalRoutingTree<IntPoint>& tree,
        std::optional<std::vector<typename GlobalRoutingTree<IntPoint>::Keepout>> keepouts
        = std::nullopt,
        const int scale_z = 100, const int width = 800, const int height = 600,
        const int margin = 50);

    template <typename IntPoint> extern void save_routing_tree3d_svg(
        const GlobalRoutingTree<IntPoint>& tree,
        std::optional<std::vector<typename GlobalRoutingTree<IntPoint>::Keepout>> keepouts
        = std::nullopt,
        const int scale_z = 100, const std::string filename = "routing_tree3d.svg",
        const int width = 800, const int height = 600);

}  // namespace recti