#include <fmt/core.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <recti/dme_algorithm.hpp>
#include <recti/logger.hpp>

namespace recti {

    // LinearDelayCalculator::calculate_tapping_point implementation
    std::pair<int, double> LinearDelayCalculator::calculate_tapping_point(TreeNode& node_left,
                                                                          TreeNode& node_right,
                                                                          int distance) {
        double skew = node_right.delay - node_left.delay;
        int extend_left = static_cast<int>(std::round((skew / delay_per_unit + distance) / 2));
        double delay_left = node_left.delay + extend_left * delay_per_unit;

        node_left.wire_length = extend_left;
        node_right.wire_length = distance - extend_left;

        if (extend_left < 0) {
            node_left.wire_length = 0;
            node_right.wire_length = distance - extend_left;
            extend_left = 0;
            delay_left = node_left.delay;
            node_right.need_elongation = true;
            log_with_spdlog(
                "Warning: Right node needs elongation: extend_left < 0  => extend_left set to "
                "0");
        } else if (extend_left > distance) {
            node_right.wire_length = 0;
            node_left.wire_length = extend_left;
            extend_left = distance;
            delay_left = node_right.delay;
            node_left.need_elongation = true;
            log_with_spdlog(
                "Warning: Left node needs elongation: extend_left > distance => extend_left "
                "set to distance");
        }

        return {extend_left, delay_left};
    }

    // ElmoreDelayCalculator::calculate_tapping_point implementation
    std::pair<int, double> ElmoreDelayCalculator::calculate_tapping_point(TreeNode& node_left,
                                                                          TreeNode& node_right,
                                                                          int distance) {
        double skew = node_right.delay - node_left.delay;
        double r = distance * unit_resistance;
        double c = distance * unit_capacitance;

        double z = (skew + r * (node_right.capacitance + c / 2.0))
                   / (r * (c + node_right.capacitance + node_left.capacitance));

        int extend_left = static_cast<int>(std::round(z * distance));
        double r_left = extend_left * unit_resistance;
        double c_left = extend_left * unit_capacitance;
        double delay_left = node_left.delay + r_left * (c_left / 2.0 + node_left.capacitance);

        node_left.wire_length = extend_left;
        node_right.wire_length = distance - extend_left;

        if (extend_left < 0) {
            node_left.wire_length = 0;
            node_right.wire_length = distance - extend_left;
            extend_left = 0;
            delay_left = node_left.delay;
            node_right.need_elongation = true;
        } else if (extend_left > distance) {
            node_right.wire_length = 0;
            node_left.wire_length = extend_left;
            extend_left = distance;
            delay_left = node_right.delay;
            node_left.need_elongation = true;
        }

        return {extend_left, delay_left};
    }

    /**
     * @brief Builds the prescribed-skew (not necessarily zero) clock tree for the given sinks.
     *
     * This is the main function that orchestrates the three phases of the DME algorithm:
     * topology construction, merging segment computation, and top-down embedding.
     * It also computes the final delays and wire lengths after embedding.
     * @return A shared pointer to the root of the constructed and embedded clock tree.
     */
    std::shared_ptr<TreeNode> DMEAlgorithm::build_clock_tree() {
        // Step 1: Create initial leaf nodes from the provided sinks.
        std::vector<std::shared_ptr<TreeNode>> nodes;
        for (const auto& sink : this->sinks) {
            auto node = std::make_shared<TreeNode>(sink.name, sink.position);
            node->capacitance = sink.capacitance;
            nodes.push_back(node);
        }
        // Step 2: Build the merging tree topology using a balanced bipartitioning strategy.
        // The 'false' argument indicates starting with a horizontal cut.
        auto merging_tree = this->build_merging_tree(nodes, false);
        // Step 3: Perform bottom-up computation of merging segments for all nodes.
        // These segments represent the possible locations for parent nodes to achieve zero
        // skew.
        std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>> merging_segments
            = this->compute_merging_segments(merging_tree);
        // Step 4: Perform top-down embedding to select the actual physical positions
        // for internal nodes within their merging segments.
        auto clock_tree = this->embed_tree(merging_tree, merging_segments);
        // Step 5: Compute the final delays and wire lengths for all nodes in the embedded tree.
        this->compute_tree_parameters(clock_tree);
        return clock_tree;
    }

    /**
     * @brief Recursively builds a balanced binary merging tree topology.
     *
     * This function partitions the given set of nodes into two balanced groups based on their
     * coordinates (alternating between x and y axes) and recursively builds subtrees.
     * @param nodes A vector of shared pointers to TreeNodes to be partitioned.
     * @param vertical A boolean flag. If true, partition along the x-axis (vertical cut);
     *                 if false, partition along the y-axis (horizontal cut).
     * @return A shared pointer to the root of the constructed merging subtree.
     */
    std::shared_ptr<TreeNode> DMEAlgorithm::build_merging_tree(
        const std::vector<std::shared_ptr<TreeNode>>& nodes, bool vertical) {
        if (nodes.size() == 1) {
            return nodes[0];
        }
        // Sort nodes along the appropriate axis to enable balanced bipartitioning.
        auto sorted_nodes = nodes;
        if (vertical) {
            std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](const auto& a, const auto& b) {
                return a->position.xcoord() < b->position.xcoord();
            });
        } else {
            std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](const auto& a, const auto& b) {
                return a->position.ycoord() < b->position.ycoord();
            });
        }
        // Split the sorted nodes into two approximately equal-sized groups.
        size_t mid = sorted_nodes.size() / 2;
        std::vector<std::shared_ptr<TreeNode>> left_group(
            sorted_nodes.begin(), sorted_nodes.begin() + static_cast<std::ptrdiff_t>(mid));
        std::vector<std::shared_ptr<TreeNode>> right_group(
            sorted_nodes.begin() + static_cast<std::ptrdiff_t>(mid), sorted_nodes.end());
        // Recursively build the left and right subtrees, alternating the cut direction.
        auto left_child = build_merging_tree(left_group, !vertical);
        auto right_child = build_merging_tree(right_group, !vertical);
        // Create a new parent node for the two subtrees.
        // Its initial position is set to the left child's position (will be updated during
        // embedding).
        auto parent
            = std::make_shared<TreeNode>(fmt::format("n{}", this->node_id++), left_child->position);
        parent->left = left_child;
        parent->right = right_child;
        // Establish parent-child relationships.
        left_child->parent = parent;
        right_child->parent = parent;
        return parent;
    }

    /**
     * @brief Computes the merging segments for all nodes in a bottom-up manner.
     *
     * This function traverses the merging tree from leaves up to the root. For each node,
     * it computes its merging segment, which is a Manhattan arc representing the locus of
     * all possible tapping points that achieve prescribed-skew (not necessarily zero) to its
     * descendant sinks.
     * @param root The root of the merging tree topology.
     * @return A map where keys are node names and values are their corresponding ManhattanArc
     * merging segments.
     * @throws std::runtime_error if an internal node does not have both left and right
     * children.
     */
    std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>>
    DMEAlgorithm::compute_merging_segments(const std::shared_ptr<TreeNode>& root) {
        std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>> merging_segments;
        // Define a recursive lambda function for computing segments.
        std::function<ManhattanArc<Interval<int>, Interval<int>>(const std::shared_ptr<TreeNode>&)>
            compute_segment;
        compute_segment = [&](const std::shared_ptr<TreeNode>& node)
            -> ManhattanArc<Interval<int>, Interval<int>> {
            if (node->is_leaf()) {
                // For a leaf node (sink), its merging segment is a point (its own position).
                auto ms1 = ManhattanArc<int, int>::from_point(node->position);
                ManhattanArc<Interval<int>, Interval<int>> ms(
                    Interval{ms1.impl.xcoord(), ms1.impl.xcoord()},
                    Interval{ms1.impl.ycoord(), ms1.impl.ycoord()});
                merging_segments[node->name] = ms;
                return ms;
            }
            // Ensure internal nodes have two children.
            if (!node->left || !node->right) {
                throw std::runtime_error("Internal node must have both left and right children");
            }
            // Recursively compute merging segments for child nodes.
            auto left_ms = compute_segment(node->left);
            auto right_ms = compute_segment(node->right);
            // Calculate the Manhattan distance between the two child merging segments.
            int distance = left_ms.min_dist_with(right_ms);
            // Calculate the optimal tapping point on the merging segment and the delay to it.
            auto [extend_left, delay_left] = this->delay_calculator->calculate_tapping_point(
                *node->left, *node->right, distance);
            node->delay = delay_left;  // Store the delay at this merging point.
            // Merge the child segments to form the current node's merging segment.
            auto merged_segment = left_ms.merge_with(right_ms, extend_left);
            merging_segments[node->name] = merged_segment;
            // Update the capacitance of the current node by summing child capacitances and wire
            // capacitance.
            double wire_cap = this->delay_calculator->calculate_wire_capacitance(distance);
            node->capacitance = node->left->capacitance + node->right->capacitance + wire_cap;
            return merged_segment;
        };
        compute_segment(root);  // Start the recursive computation from the root.
        return merging_segments;
    }

    /**
     * @brief Performs top-down embedding to determine the physical locations of internal nodes.
     *
     * This function traverses the tree from the root down to the leaves. For each internal
     * node, it selects a specific point within its merging segment to be its physical position.
     * The root's position is typically chosen as the upper corner of its merging segment.
     * For other internal nodes, the position is chosen to be the nearest point on its merging
     * segment to its parent's already determined position.
     * @param merging_tree The root of the merging tree topology.
     * @param merging_segments A map containing the pre-computed ManhattanArc merging segments
     * for all nodes.
     * @return The root of the embedded clock tree with concrete physical positions.
     * @throws std::runtime_error if a merging segment is not found for a node.
     */
    std::shared_ptr<TreeNode> DMEAlgorithm::embed_tree(
        const std::shared_ptr<TreeNode>& merging_tree,
        const std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>>& merging_segments) {
        // Define a recursive lambda function for embedding nodes.
        std::function<void(const std::shared_ptr<TreeNode>&,
                           const ManhattanArc<Interval<int>, Interval<int>>*)>
            embed_node;
        embed_node = [&](const std::shared_ptr<TreeNode>& node,
                         const ManhattanArc<Interval<int>, Interval<int>>* parent_segment) {
            if (!node) return;
            // Retrieve the merging segment for the current node.
            auto it = merging_segments.find(node->name);
            if (it == merging_segments.end()) {
                throw std::runtime_error("Merging segment not found for node: " + node->name);
            }
            const auto& node_segment = it->second;
            if (!parent_segment) {
                // For the root node, choose the upper corner of its merging segment as its
                // position.
                node->position = node_segment.get_upper_corner();
            } else {
                // For internal nodes, choose the point on its merging segment nearest to its
                // parent's position.
                node->position = node_segment.nearest_point_to(node->parent->position);
                // Calculate the wire length to the parent.
                if (node->parent) {
                    node->wire_length = node->position.min_dist_with(node->parent->position);
                }
            }
            // Recursively embed the children nodes.
            embed_node(node->left, &node_segment);
            embed_node(node->right, &node_segment);
        };
        embed_node(merging_tree,
                   nullptr);  // Start embedding from the root with no parent segment.
        return merging_tree;
    }

    /**
     * @brief Computes the final delays and wire lengths for all nodes in the embedded tree.
     *
     * This function performs a top-down traversal of the embedded clock tree to calculate
     * the accumulated delay from the root to each node and the wire length from each node
     * to its parent. The root node is assumed to have zero delay.
     * @param root The root of the embedded clock tree.
     */
    void DMEAlgorithm::compute_tree_parameters(const std::shared_ptr<TreeNode>& root) {
        // Define a recursive lambda function for computing delays.
        std::function<void(const std::shared_ptr<TreeNode>&, double)> compute_delays;
        compute_delays = [&](const std::shared_ptr<TreeNode>& node, double parent_delay) {
            if (!node) return;
            if (node->parent) {
                // Calculate wire delay from parent to current node and accumulate total delay.
                double wire_delay = this->delay_calculator->calculate_wire_delay(node->wire_length,
                                                                                 node->capacitance);
                node->delay = parent_delay + wire_delay;
            } else {
                // The root node has zero delay.
                node->delay = 0.0;
            }
            // Recursively compute delays for children.
            compute_delays(node->left, node->delay);
            compute_delays(node->right, node->delay);
        };
        compute_delays(root,
                       0.0);  // Start delay computation from the root with initial delay 0.
    }

    /**
     * @brief Analyzes the clock skew of the constructed tree.
     *
     * This function traverses the clock tree to collect delays at all sink nodes,
     * then calculates the maximum delay, minimum delay, and the overall skew.
     * It also reports the total wirelength and the delay model used.
     * @param root The root of the clock tree.
     * @return A SkewAnalysis struct containing the analysis results.
     * @throws std::runtime_error if no sink delays are collected (e.g., empty tree).
     */
    SkewAnalysis DMEAlgorithm::analyze_skew(const std::shared_ptr<TreeNode>& root) const {
        std::vector<double> sink_delays;  // Stores delays to all sink nodes.
        // Define a recursive lambda function to collect sink delays.
        std::function<void(const std::shared_ptr<TreeNode>&)> collect_sink_delays;
        collect_sink_delays = [&](const std::shared_ptr<TreeNode>& node) {
            if (!node) return;
            if (node->is_leaf()) {
                sink_delays.push_back(node->delay);
            }
            collect_sink_delays(node->left);
            collect_sink_delays(node->right);
        };
        collect_sink_delays(root);  // Start collecting sink delays from the root.
        if (sink_delays.empty()) {
            throw std::runtime_error("No sink delays collected");
        }
        // Calculate max, min, and skew from collected sink delays.
        double max_delay = *std::max_element(sink_delays.begin(), sink_delays.end());
        double min_delay = *std::min_element(sink_delays.begin(), sink_delays.end());
        double skew = max_delay - min_delay;
        // Return the comprehensive skew analysis results.
        const auto& calculator = *this->delay_calculator;
        return {max_delay,
                min_delay,
                skew,
                sink_delays,
                this->total_wirelength(root),
                typeid(calculator).name()};
    }

    /**
     * @brief Calculates the total wirelength of the clock tree.
     *
     * This private helper function traverses the tree and sums up the `wire_length`
     * attribute of each node to get the total wirelength of the clock tree.
     * @param root The root of the clock tree.
     * @return The total wirelength of the clock tree.
     */
    int DMEAlgorithm::total_wirelength(const std::shared_ptr<TreeNode>& root) const {
        int total = 0;
        // Define a recursive lambda function to sum wire lengths.
        std::function<void(const std::shared_ptr<TreeNode>&)> sum_wirelength;
        sum_wirelength = [&](const std::shared_ptr<TreeNode>& node) {
            if (!node) return;
            total += node->wire_length;
            sum_wirelength(node->left);
            sum_wirelength(node->right);
        };
        sum_wirelength(root);  // Start summing from the root.
        return total;
    }

    // get_tree_statistics implementation
    TreeStatistics get_tree_statistics(const std::shared_ptr<TreeNode>& root) {
        TreeStatistics stats;
        std::function<void(const std::shared_ptr<TreeNode>&, const std::shared_ptr<TreeNode>&)>
            traverse;

        traverse
            = [&](const std::shared_ptr<TreeNode>& node, const std::shared_ptr<TreeNode>& parent) {
                  if (!node) return;

                  stats.nodes.push_back({node->name,
                                         {node->position.xcoord(), node->position.ycoord()},
                                         node->is_leaf() ? "sink" : "internal",
                                         node->delay,
                                         node->capacitance});

                  if (node->is_leaf()) {
                      stats.sinks.push_back(node->name);
                  }

                  if (parent) {
                      stats.wires.push_back({parent->name,
                                             node->name,
                                             node->wire_length,
                                             {parent->position.xcoord(), parent->position.ycoord()},
                                             {node->position.xcoord(), node->position.ycoord()}});
                  }

                  traverse(node->left, node);
                  traverse(node->right, node);
              };

        traverse(root, nullptr);

        stats.total_nodes = static_cast<int>(stats.nodes.size());
        stats.total_sinks = static_cast<int>(stats.sinks.size());
        stats.total_wires = static_cast<int>(stats.wires.size());

        return stats;
    }

}  // namespace recti

/**
 * @brief Example usage function demonstrating the DME algorithm with different delay models.
 *
 * This function sets up a sample clock tree synthesis problem with multiple sinks,
 * applies the DME algorithm using both Linear and Elmore delay models,
 * and prints the skew analysis results for each.
 */
void example_dme_usage() {
    using namespace recti;

    std::cout << "=== Linear Delay Model ===" << std::endl;

    // Define a set of clock sinks with their positions and capacitances.
    std::vector<Sink> sinks
        = {Sink("s1", Point<int>(10, 20), 1.0), Sink("s2", Point<int>(30, 40), 1.0),
           Sink("s3", Point<int>(50, 10), 1.0), Sink("s4", Point<int>(70, 30), 1.0),
           Sink("s5", Point<int>(90, 50), 1.0)};

    // Create a LinearDelayCalculator and run the DME algorithm.
    auto linear_calc = std::make_unique<LinearDelayCalculator>(0.5, 0.2);
    DMEAlgorithm dme_linear(sinks, std::move(linear_calc));
    auto clock_tree_linear = dme_linear.build_clock_tree();
    auto analysis_linear = dme_linear.analyze_skew(clock_tree_linear);

    // Print results for the Linear Delay Model.
    std::cout << "Delay Model: " << analysis_linear.delay_model << std::endl;
    std::cout << "Maximum delay: " << analysis_linear.max_delay << std::endl;
    std::cout << "Minimum delay: " << analysis_linear.min_delay << std::endl;
    std::cout << "Clock skew: " << analysis_linear.skew << std::endl;
    std::cout << "Total wirelength: " << analysis_linear.total_wirelength << std::endl;

    std::cout << "\n=== Elmore Delay Model ===" << std::endl;

    // Create an ElmoreDelayCalculator and run the DME algorithm.
    auto elmore_calc = std::make_unique<ElmoreDelayCalculator>(0.1, 0.2);
    DMEAlgorithm dme_elmore(sinks, std::move(elmore_calc));
    auto clock_tree_elmore = dme_elmore.build_clock_tree();
    auto analysis_elmore = dme_elmore.analyze_skew(clock_tree_elmore);

    // Print results for the Elmore Delay Model.
    std::cout << "Delay Model: " << analysis_elmore.delay_model << std::endl;
    std::cout << "Maximum delay: " << analysis_elmore.max_delay << std::endl;
    std::cout << "Minimum delay: " << analysis_elmore.min_delay << std::endl;
    std::cout << "Clock skew: " << analysis_elmore.skew << std::endl;
    std::cout << "Total wirelength: " << analysis_elmore.total_wirelength << std::endl;
}
