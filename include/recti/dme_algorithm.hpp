#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "manhattan_arc.hpp"
#include "point.hpp"

namespace recti {

    // Forward declarations
    class Tree;

    /**
     * @brief Node index type used throughout the DME algorithm.
     *
     * Nodes are stored in an arena-allocated `Tree` and referenced by
     * `NodeIdx` (like Rust's `usize`).  `SIZE_MAX` serves as the null
     * sentinel (equivalent to Rust's `None`).
     */
    using NodeIdx = std::size_t;

    /**
     * @class Sink
     * @brief Represents a clock sink in the clock tree network.
     *
     * A sink is a terminal point in the clock tree that requires a clock signal.
     * It is defined by its name, physical position, and electrical capacitance.
     */
    class Sink {
      public:
        std::string name;
        Point<int> position;
        double capacitance;

        /**
         * @brief Constructs a new Sink object.
         * @param name The name of the sink.
         * @param position The physical coordinates of the sink.
         * @param capacitance The load capacitance of the sink. Defaults to 1.0.
         */
        constexpr Sink(const std::string& name, const Point<int>& position,
                       double capacitance = 1.0)
            : name(name), position(position), capacitance(capacitance) {}
    };

    /**
     * @class TreeNode
     * @brief A node in the clock tree, stored in a `Tree` arena.
     *
     * Connectivity is expressed with `NodeIdx` indices into the owning
     * `Tree` rather than raw / smart pointers.  `SIZE_MAX` means "no
     * such neighbour".
     */
    class TreeNode {
      public:
        std::string name;
        Point<int> position;
        NodeIdx left = SIZE_MAX;    ///< Index of the left child (or SIZE_MAX)
        NodeIdx right = SIZE_MAX;   ///< Index of the right child (or SIZE_MAX)
        NodeIdx parent = SIZE_MAX;  ///< Index of the parent (or SIZE_MAX)
        int wire_length = 0;        ///< Length of the wire connecting this node to its parent.
        double delay = 0.0;         ///< Accumulated delay from the clock source to this node.
        double capacitance = 0.0;   ///< Total downstream capacitance seen from this node.
        bool need_elongation
            = false;  ///< Flag indicating if a branch needed elongation for skew balancing.

        /**
         * @brief Constructs a new TreeNode.
         * @param name The unique name of the node.
         * @param position The physical coordinates of the node in the grid.
         */
        TreeNode(const std::string& name, const Point<int>& position)
            : name(name), position(position) {}

        /**
         * @brief Checks if the node is a leaf (a sink).
         * @return True if the node has no children, false otherwise.
         */
        bool is_leaf() const { return left == SIZE_MAX && right == SIZE_MAX; }
    };

    /**
     * @class Tree
     * @brief Arena-allocated tree of `TreeNode`s.
     *
     * Nodes are stored in a contiguous `std::vector` and referenced by
     * their `NodeIdx` index.  This avoids `std::shared_ptr` overhead,
     * improves cache locality, and allows safe simultaneous mutation
     * of two distinct nodes via `get_pair_mut`.
     */
    class Tree {
      private:
        std::vector<TreeNode> nodes;

      public:
        /** Index of the root node, or SIZE_MAX if the tree is empty. */
        NodeIdx root = SIZE_MAX;

        Tree() = default;

        /**
         * @brief Adds a node to the arena and returns its index.
         * @param node The node to add.
         * @return The index of the newly added node.
         */
        NodeIdx add(TreeNode node) {
            NodeIdx idx = nodes.size();
            nodes.push_back(std::move(node));
            return idx;
        }

        /**
         * @brief Returns a const reference to the node at the given index.
         */
        const TreeNode& get(NodeIdx idx) const { return nodes.at(idx); }

        /**
         * @brief Returns a mutable reference to the node at the given index.
         */
        TreeNode& get_mut(NodeIdx idx) { return nodes.at(idx); }

        /**
         * @brief Simultaneously returns mutable references to two distinct nodes.
         *
         * Safe because distinct vector elements are independent in C++.
         * @throws std::out_of_range if either index is out of bounds.
         */
        std::pair<TreeNode&, TreeNode&> get_pair_mut(NodeIdx a, NodeIdx b) {
            return {nodes.at(a), nodes.at(b)};
        }

        /** @brief Number of nodes in the arena. */
        std::size_t size() const { return nodes.size(); }

        /** @brief True when the arena holds no nodes. */
        bool empty() const { return nodes.empty(); }
    };

    /**
     * @brief Result of a tapping-point calculation.
     *
     * Carries both the clamped `extend_left` (the tapping point along the
     * merging interval, guaranteed to lie in [0, distance]) and the
     * **raw** (pre-clamp) value so that the caller can compute the exact
     * wire lengths for the two children as well as the `need_elongation`
     * flags.
     */
    struct TappingResult {
        /** Tapping-point offset from the left child, clamped to [0, distance]. */
        int extend_left;
        /** Raw (pre-clamp) tapping-point offset. */
        int raw_extend_left;
        /** Signal delay at the tapping point. */
        double delay_left;
    };

    /**
     * @class DelayCalculator
     * @brief Abstract base class for delay calculation models.
     *
     * This class defines the interface for different delay models (e.g., Linear,
     * Elmore) used in the DME algorithm to estimate wire delays and capacitances.
     *
     * `calculate_tapping_point` is a **pure** function that takes scalar values
     * and returns a `TappingResult`.  All node mutation side effects
     * (setting `wire_length`, `need_elongation`) are delegated to the caller.
     */
    class DelayCalculator {
      public:
        DelayCalculator() = default;
        DelayCalculator(const DelayCalculator&) = default;
        DelayCalculator& operator=(const DelayCalculator&) = default;
        DelayCalculator(DelayCalculator&&) = default;
        DelayCalculator& operator=(DelayCalculator&&) = default;
        virtual ~DelayCalculator() = default;

        /**
         * @brief Calculates the delay of a wire segment.
         * @param length The length of the wire segment.
         * @param load_capacitance The total capacitance driven by this wire segment.
         * @return The calculated wire delay in time units.
         */
        virtual double calculate_wire_delay(int length, double load_capacitance) const = 0;

        /**
         * @brief Calculates the delay per unit length of a wire.
         * @param load_capacitance The total capacitance driven by this wire segment.
         * @return The delay per unit length in time units per length unit.
         */
        virtual double calculate_wire_delay_per_unit(double load_capacitance) const = 0;

        /**
         * @brief Calculates the capacitance of a wire segment.
         * @param length The length of the wire.
         * @return The calculated wire capacitance.
         */
        virtual double calculate_wire_capacitance(int length) const = 0;

        /**
         * @brief Determines the optimal tapping point for merging two subtrees to
         *        achieve prescribed skew.
         *
         * This pure method computes where a new parent node should be placed on
         * the merging segment between two child nodes to balance their delays.
         * It returns a `TappingResult` that contains both the clamped `extend_left`
         * (in [0, distance]) and the **raw** pre-clamp value, enabling the caller
         * to implement elongation logic (when the raw value falls outside [0,
         * distance]).
         *
         * @param distance The Manhattan distance between the merging segments.
         * @param left_delay  The delay at the left child.
         * @param right_delay The delay at the right child.
         * @param left_capacitance  The capacitance of the left subtree.
         * @param right_capacitance The capacitance of the right subtree.
         * @return A `TappingResult` containing the clamped `extend_left`, the
         *         raw `raw_extend_left`, and the resulting `delay_left`.
         */
        virtual TappingResult calculate_tapping_point(int distance, double left_delay,
                                                      double right_delay, double left_capacitance,
                                                      double right_capacitance) const
            = 0;
    };

    /**
     * @class LinearDelayCalculator
     * @brief Implements a simple linear delay model.
     *
     * In this model, delay is directly proportional to the wire length.
     * Delay = delay_per_unit * length.
     * Capacitance = capacitance_per_unit * length.
     */
    class LinearDelayCalculator : public DelayCalculator {
      private:
        double delay_per_unit;        ///< Delay constant per unit of wire length.
        double capacitance_per_unit;  ///< Capacitance constant per unit of wire length.

      public:
        LinearDelayCalculator(double delay_per_unit = 1.0, double capacitance_per_unit = 1.0)
            : delay_per_unit(delay_per_unit), capacitance_per_unit(capacitance_per_unit) {}

        double calculate_wire_delay(int length, double /* load_capacitance */) const override {
            return delay_per_unit * static_cast<double>(length);
        }

        double calculate_wire_delay_per_unit(double /* load_capacitance */) const override {
            return delay_per_unit;
        }

        double calculate_wire_capacitance(int length) const override {
            return capacitance_per_unit * static_cast<double>(length);
        }

        /**
         * @brief Linear-model tapping point.
         *
         * `extend_left = round((skew / delay_per_unit + distance) / 2)`
         *
         * When the raw `extend_left` falls below 0 or above `distance`, the value
         * is clamped and `delay_left` is set to the delay of the opposite child
         * (the one that does *not* need elongation).  The raw value is preserved in
         * `raw_extend_left` so that the caller can apply the original elongation
         * wire-length rules.
         */
        TappingResult calculate_tapping_point(int distance, double left_delay, double right_delay,
                                              double /* left_capacitance */,
                                              double /* right_capacitance */) const override;
    };

    /**
     * @class ElmoreDelayCalculator
     * @brief Implements the Elmore delay model for RC trees.
     *
     * The Elmore delay model is a widely used approximation for delay in RC
     * interconnects.  It considers both the resistance and capacitance of the wires
     * and the downstream load.
     * Delay = R_wire * (C_wire/2 + C_load).
     */
    class ElmoreDelayCalculator : public DelayCalculator {
      private:
        double unit_resistance;   ///< Resistance per unit of wire length.
        double unit_capacitance;  ///< Capacitance per unit of wire length.

      public:
        ElmoreDelayCalculator(double unit_resistance = 1.0, double unit_capacitance = 1.0)
            : unit_resistance(unit_resistance), unit_capacitance(unit_capacitance) {}

        double calculate_wire_delay(int length, double load_capacitance) const override {
            double wire_resistance = unit_resistance * length;
            double wire_capacitance = unit_capacitance * length;
            return wire_resistance * (wire_capacitance / 2 + load_capacitance);
        }

        double calculate_wire_delay_per_unit(double load_capacitance) const override {
            return unit_resistance * (unit_capacitance / 2 + load_capacitance);
        }

        double calculate_wire_capacitance(int length) const override {
            return unit_capacitance * length;
        }

        /**
         * @brief Elmore-model tapping point.
         *
         * Solves for the optimal tapping point `z` (fraction of distance from left)
         * to achieve prescribed skew between the children considering their delays
         * and capacitances:
         *
         *   z = (skew + R*(C_r + C_w/2)) / (R*(C_w + C_r + C_l))
         *
         * where R = distance * unit_resistance, C_w = distance * unit_capacitance.
         * When the raw `extend_left` falls below 0 or above `distance`, clamping
         * and elongation logic applies (see `LinearDelayCalculator`).
         */
        TappingResult calculate_tapping_point(int distance, double left_delay, double right_delay,
                                              double left_capacitance,
                                              double right_capacitance) const override;
    };

    /**
     * @struct SkewAnalysis
     * @brief Stores the results of the clock skew analysis for a clock tree.
     */
    struct SkewAnalysis {
        double max_delay;                 ///< The maximum delay from the clock source to any sink.
        double min_delay;                 ///< The minimum delay from the clock source to any sink.
        double skew;                      ///< The difference between max_delay and min_delay.
        std::vector<double> sink_delays;  ///< A list of delays to each individual sink.
        int total_wirelength;             ///< The sum of all wire lengths in the clock tree.
        std::string
            delay_model;  ///< The name of the delay model used (e.g. "LinearDelayCalculator").
    };

    /**
     * @struct TreeStatistics
     * @brief Provides detailed statistics about the generated clock tree.
     */
    struct TreeStatistics {
        struct NodeInfo {
            std::string name;              ///< Name of the node.
            std::pair<int, int> position;  ///< (x, y) coordinates.
            std::string type;              ///< Type: "sink" or "internal".
            double delay;                  ///< Accumulated delay.
            double capacitance;            ///< Downstream capacitance.
        };

        struct WireInfo {
            std::string from_node;         ///< Source node name.
            std::string to_node;           ///< Destination node name.
            int length;                    ///< Wire length.
            std::pair<int, int> from_pos;  ///< Source (x, y).
            std::pair<int, int> to_pos;    ///< Destination (x, y).
        };

        std::vector<NodeInfo> nodes;
        std::vector<WireInfo> wires;
        std::vector<std::string> sinks;
        int total_nodes = 0;
        int total_sinks = 0;
        int total_wires = 0;
    };

    /**
     * @class DMEAlgorithm
     * @brief Implements the Deferred Merge Embedding (DME) algorithm for clock tree
     *        synthesis.
     *
     * Uses an arena-allocated node representation (`Tree`) for cache efficiency.
     * Supports both linear and Elmore delay models via the `DelayCalculator` interface.
     *
     * **Three phases:**
     * 1. **Topology construction** (`build_merging_tree`): balanced bipartitioning by
     *    coordinates.
     * 2. **Bottom-up merging** (`compute_merging_segments`): compute Manhattan arc merging
     *    segments.
     * 3. **Top-down embedding** (`embed_tree`): select physical positions for internal nodes.
     */
    class DMEAlgorithm {
      private:
        std::vector<Sink> sinks;
        std::unique_ptr<DelayCalculator> delay_calculator;
        int node_id = 0;
        Tree tree;

      public:
        /**
         * @brief Constructs a DMEAlgorithm object.
         * @param sinks A vector of clock sinks.
         * @param calculator A unique pointer to a DelayCalculator implementation.
         * @throws std::invalid_argument if sinks is empty.
         */
        DMEAlgorithm(const std::vector<Sink>& sinks, std::unique_ptr<DelayCalculator> calculator)
            : sinks(sinks), delay_calculator(std::move(calculator)) {
            if (this->sinks.empty()) {
                throw std::invalid_argument("No sinks provided");
            }
        }

        /**
         * @brief Builds the clock tree for the given sinks.
         *
         * Orchestrates the three phases of the DME algorithm: topology construction,
         * merging segment computation, and top-down embedding.  Also computes the
         * final delays and wire lengths after embedding.
         * @return The root `NodeIdx` of the constructed and embedded clock tree.
         */
        NodeIdx build_clock_tree();

        /** @brief Returns a const reference to the internal tree arena. */
        const Tree& get_tree() const { return tree; }

        /** @brief Returns a mutable reference to the internal tree arena. */
        Tree& get_tree_mut() { return tree; }

        /**
         * @brief Analyzes clock skew of the constructed tree.
         *
         * Traverses the clock tree to collect delays at all sink nodes, then calculates
         * the maximum delay, minimum delay, and the overall skew.  Also reports the
         * total wirelength and the delay model used.
         * @param root The root index of the clock tree.
         * @return A SkewAnalysis struct containing the analysis results.
         * @throws std::runtime_error if no sink delays are collected (e.g., empty tree).
         */
        SkewAnalysis analyze_skew(NodeIdx root) const;

      private:
        /**
         * @brief Recursively builds a balanced binary merging tree topology.
         *
         * Partitions the given set of node indices into two balanced groups based on
         * their coordinates (alternating between x and y axes) and recursively builds
         * subtrees.
         * @param node_ids The indices of nodes to partition.
         * @param vertical If true, partition along the x-axis (vertical cut);
         *                 otherwise along the y-axis (horizontal cut).
         * @return The root index of the constructed merging subtree.
         */
        NodeIdx build_merging_tree(const std::vector<NodeIdx>& node_ids, bool vertical);

        /**
         * @brief Recursive helper for merging segment computation.
         *
         * For a leaf node, creates a zero-width ManhattanArc at its position.
         * For an internal node, recursively computes child segments, determines the
         * tapping point via the delay calculator, then merges the child segments.
         * Also updates `wire_length`, `need_elongation`, `delay`, and `capacitance`
         * on the nodes involved, including the full elongation logic (raw extend_left).
         */
        ManhattanArc<Interval<int>, Interval<int>> _compute_merging_segment(
            NodeIdx node, std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>&
                              merging_segments);

        /**
         * @brief Computes merging segments for all nodes bottom-up.
         */
        std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>
        compute_merging_segments(NodeIdx root);

        /**
         * @brief Recursive helper for top-down embedding.
         *
         * For the root node, picks the upper corner of its merging segment.
         * For other nodes, picks the point on its merging segment nearest to its
         * parent and updates `wire_length`.
         */
        void _embed_node(
            NodeIdx node, const ManhattanArc<Interval<int>, Interval<int>>* parent_segment,
            const std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>&
                merging_segments);

        /**
         * @brief Performs top-down embedding to determine the physical locations of
         *        internal nodes.
         * @return The root of the embedded tree (same index as input).
         */
        NodeIdx embed_tree(
            NodeIdx merging_tree_root,
            const std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>&
                merging_segments);

        /**
         * @brief Recursive helper for delay computation.
         */
        void _compute_delays(NodeIdx node, double parent_delay);

        /**
         * @brief Computes final delays and wire lengths for all nodes.
         */
        void compute_tree_parameters(NodeIdx root);

        /**
         * @brief Calculates total wirelength of the clock tree.
         * @return The sum of all `wire_length` values.
         */
        int total_wirelength(NodeIdx root) const;
    };

    /**
     * @brief Extracts detailed statistics from a constructed clock tree.
     *
     * Traverses the clock tree and gathers comprehensive information about its nodes,
     * wires, and overall structure, populating a TreeStatistics object.
     * @param tree The arena-allocated tree.
     * @param root The root node index.
     */
    TreeStatistics get_tree_statistics(const Tree& tree, NodeIdx root);

    /**
     * @brief Example usage function demonstrating the DME algorithm with different
     *        delay models.
     *
     * Sets up a sample clock tree synthesis problem with multiple sinks, applies the
     * DME algorithm using both Linear and Elmore delay models, and prints the skew
     * analysis results for each.
     */
    extern void example_dme_usage();

}  // namespace recti
