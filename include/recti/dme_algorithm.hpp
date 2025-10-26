#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Assuming the header files are included
#include "manhattan_arc.hpp"
#include "point.hpp"

namespace recti {

    // Forward declarations
    class TreeNode;

    /**
     * @brief Represents a clock sink with position and capacitance
     */
    class Sink {
      public:
        std::string name;
        Point<int> position;
        double capacitance;

        /**
         * @brief Construct a new Sink object
         */
        constexpr Sink(const std::string& name, const Point<int>& position,
                       double capacitance = 1.0)
            : name(name), position(position), capacitance(capacitance) {}
    };

    /**
     * @brief Abstract base class for delay calculation strategies
     */
    class DelayCalculator {
      public:
        virtual ~DelayCalculator() = default;

        /**
         * @brief Calculate wire delay for given length and load capacitance
         */
        virtual double calculate_wire_delay(int length, double load_capacitance) = 0;

        /**
         * @brief Calculate delay per unit length for given load capacitance
         */
        virtual double calculate_wire_delay_per_unit(double load_capacitance) = 0;

        /**
         * @brief Calculate wire capacitance for given length
         */
        virtual double calculate_wire_capacitance(int length) = 0;

        /**
         * @brief Calculate tapping point based on skew
         */
        virtual std::pair<int, double> calculate_tapping_point(TreeNode& node_left,
                                                               TreeNode& node_right, int distance)
            = 0;
    };

    /**
     * @brief Represents a node in the clock tree
     */
    class TreeNode {
      public:
        std::string name;
        Point<int> position;
        std::shared_ptr<TreeNode> left;
        std::shared_ptr<TreeNode> right;
        std::shared_ptr<TreeNode> parent;
        int wire_length = 0;
        double delay = 0.0;
        double capacitance = 0.0;
        bool need_elongation = false;

        /**
         * @brief Construct a new TreeNode object
         */
        TreeNode(const std::string& name, const Point<int>& position)
            : name(name), position(position) {}

        /**
         * @brief Check if node is a leaf (sink)
         */
        bool is_leaf() const { return !left && !right; }
    };

    /**
     * @brief Linear delay model: delay = k * length
     */
    class LinearDelayCalculator : public DelayCalculator {
      private:
        double delay_per_unit;
        double capacitance_per_unit;

      public:
        /**
         * @brief Initialize linear delay calculator
         */
        LinearDelayCalculator(double delay_per_unit = 1.0, double capacitance_per_unit = 1.0)
            : delay_per_unit(delay_per_unit), capacitance_per_unit(capacitance_per_unit) {}

        double calculate_wire_delay(int length, double) override {
            return delay_per_unit * static_cast<double>(length);
        }

        double calculate_wire_delay_per_unit(double) override { return delay_per_unit; }

        double calculate_wire_capacitance(int length) override {
            return capacitance_per_unit * static_cast<double>(length);
        }

        std::pair<int, double> calculate_tapping_point(TreeNode& node_left, TreeNode& node_right,
                                                       int distance) override {
            // Compute required delay balancing
            double skew = node_right.delay - node_left.delay;
            int extend_left = static_cast<int>(std::round((skew / delay_per_unit + distance) / 2));
            double delay_left = node_left.delay + extend_left * delay_per_unit;

            node_left.wire_length = extend_left;
            node_right.wire_length = distance - extend_left;

            if (extend_left < 0) {
                std::cout << "extend_left: " << extend_left << std::endl;
                node_left.wire_length = 0;
                node_right.wire_length = distance - extend_left;
                extend_left = 0;
                delay_left = node_left.delay;
                node_right.need_elongation = true;
            } else if (extend_left > distance) {
                std::cout << "extend_left: " << extend_left << std::endl;
                node_right.wire_length = 0;
                node_right.wire_length = extend_left;
                extend_left = distance;
                delay_left = node_right.delay;
                node_left.need_elongation = true;
            }

            return {extend_left, delay_left};
        }
    };

    /**
     * @brief Elmore delay model for RC trees
     */
    class ElmoreDelayCalculator : public DelayCalculator {
      private:
        double unit_resistance;
        double unit_capacitance;

      public:
        /**
         * @brief Initialize Elmore delay calculator
         */
        ElmoreDelayCalculator(double unit_resistance = 1.0, double unit_capacitance = 1.0)
            : unit_resistance(unit_resistance), unit_capacitance(unit_capacitance) {}

        double calculate_wire_delay(int length, double load_capacitance) override {
            double wire_resistance = unit_resistance * length;
            double wire_capacitance = unit_capacitance * length;
            // Elmore delay: R_wire * (C_wire/2 + C_load)
            return wire_resistance * (wire_capacitance / 2 + load_capacitance);
        }

        double calculate_wire_delay_per_unit(double load_capacitance) override {
            return unit_resistance * (unit_capacitance / 2 + load_capacitance);
        }

        double calculate_wire_capacitance(int length) override { return unit_capacitance * length; }

        std::pair<int, double> calculate_tapping_point(TreeNode& node_left, TreeNode& node_right,
                                                       int distance) override {
            // Compute required delay balancing
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
                std::cout << "extend_left: " << extend_left << std::endl;
                node_left.wire_length = 0;
                node_right.wire_length = distance - extend_left;
                extend_left = 0;
                delay_left = node_left.delay;
                node_right.need_elongation = true;
            } else if (extend_left > distance) {
                std::cout << "extend_left: " << extend_left << std::endl;
                node_right.wire_length = 0;
                node_right.wire_length = extend_left;
                extend_left = distance;
                delay_left = node_right.delay;
                node_left.need_elongation = true;
            }

            return {extend_left, delay_left};
        }
    };

    /**
     * @brief Analysis results structure
     */
    struct SkewAnalysis {
        double max_delay;
        double min_delay;
        double skew;
        std::vector<double> sink_delays;
        int total_wirelength;
        std::string delay_model;
    };

    /**
     * @brief Tree statistics structure
     */
    struct TreeStatistics {
        struct NodeInfo {
            std::string name;
            std::pair<int, int> position;
            std::string type;
            double delay;
            double capacitance;
        };

        struct WireInfo {
            std::string from_node;
            std::string to_node;
            int length;
            std::pair<int, int> from_pos;
            std::pair<int, int> to_pos;
        };

        std::vector<NodeInfo> nodes;
        std::vector<WireInfo> wires;
        std::vector<std::string> sinks;
        int total_nodes;
        int total_sinks;
        int total_wires;
    };

    /**
     * @brief Deferred Merge Embedding (DME) Algorithm for Clock Tree Synthesis
     */
    class DMEAlgorithm {
      private:
        std::vector<Sink> sinks;
        std::unique_ptr<DelayCalculator> delay_calculator;
        int node_id = 0;

      public:
        /**
         * @brief Initialize DME algorithm with delay calculation strategy
         */
        DMEAlgorithm(const std::vector<Sink>& sinks, std::unique_ptr<DelayCalculator> calculator)
            : sinks(sinks), delay_calculator(std::move(calculator)) {
            if (sinks.empty()) {
                throw std::invalid_argument("No sinks provided");
            }
        }

        /**
         * @brief Build a zero-skew clock tree for the given sinks
         */
        std::shared_ptr<TreeNode> build_clock_tree() {
            // Step 1: Create initial leaf nodes
            std::vector<std::shared_ptr<TreeNode>> nodes;
            for (const auto& sink : sinks) {
                auto node = std::make_shared<TreeNode>(sink.name, sink.position);
                node->capacitance = sink.capacitance;
                nodes.push_back(node);
            }

            // Step 2: Build merging tree using balanced bipartition
            auto merging_tree = build_merging_tree(nodes, false);

            // Step 3: Perform bottom-up merging segment computation
            std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>> merging_segments
                = compute_merging_segments(merging_tree);

            // Step 4: Perform top-down embedding
            auto clock_tree = embed_tree(merging_tree, merging_segments);

            // Step 5: Compute delays and wire lengths
            compute_tree_parameters(clock_tree);

            return clock_tree;
        }

      private:
        /**
         * @brief Build a balanced merging tree using recursive bipartition
         */
        std::shared_ptr<TreeNode> build_merging_tree(
            const std::vector<std::shared_ptr<TreeNode>>& nodes, bool vertical) {
            if (nodes.size() == 1) {
                return nodes[0];
            }

            // Sort nodes along the appropriate axis
            auto sorted_nodes = nodes;
            if (vertical) {
                std::sort(sorted_nodes.begin(), sorted_nodes.end(),
                          [](const auto& a, const auto& b) {
                              return a->position.xcoord() < b->position.xcoord();
                          });
            } else {
                std::sort(sorted_nodes.begin(), sorted_nodes.end(),
                          [](const auto& a, const auto& b) {
                              return a->position.ycoord() < b->position.ycoord();
                          });
            }

            // Split into balanced groups
            size_t mid = sorted_nodes.size() / 2;
            std::vector<std::shared_ptr<TreeNode>> left_group(sorted_nodes.begin(),
                                                              sorted_nodes.begin() + mid);
            std::vector<std::shared_ptr<TreeNode>> right_group(sorted_nodes.begin() + mid,
                                                               sorted_nodes.end());

            // Recursively build subtrees
            auto left_child = build_merging_tree(left_group, !vertical);
            auto right_child = build_merging_tree(right_group, !vertical);

            // Create parent node
            auto parent = std::make_shared<TreeNode>(std::string("n") + std::to_string(node_id++),
                                                     left_child->position);
            parent->left = left_child;
            parent->right = right_child;

            // Establish parent relationships
            left_child->parent = parent;
            right_child->parent = parent;

            return parent;
        }

        /**
         * @brief Compute merging segments for all nodes in bottom-up order
         */
        std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>> compute_merging_segments(
            const std::shared_ptr<TreeNode>& root) {
            std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>> merging_segments;

            std::function<ManhattanArc<Interval<int>, Interval<int>>(
                const std::shared_ptr<TreeNode>&)>
                compute_segment;
            compute_segment = [&](const std::shared_ptr<TreeNode>& node)
                -> ManhattanArc<Interval<int>, Interval<int>> {
                if (node->is_leaf()) {
                    // Leaf node: merging segment is simply its position
                    auto ms1 = ManhattanArc<int, int>::from_point(node->position);
                    ManhattanArc<Interval<int>, Interval<int>> ms(
                        Interval{ms1.impl.xcoord(), ms1.impl.xcoord()},
                        Interval{ms1.impl.ycoord(), ms1.impl.ycoord()});
                    merging_segments[node->name] = ms;
                    return ms;
                }

                if (!node->left || !node->right) {
                    throw std::runtime_error(
                        "Internal node must have both left and right children");
                }

                // Recursively compute child segments
                auto left_ms = compute_segment(node->left);
                auto right_ms = compute_segment(node->right);

                // Calculate Manhattan distance between child segments
                int distance = left_ms.min_dist_with(right_ms);

                // Calculate tapping point and delay
                auto [extend_left, delay_left] = delay_calculator->calculate_tapping_point(
                    *node->left, *node->right, distance);

                node->delay = delay_left;

                // Merge segments based on tapping point
                auto merged_segment = left_ms.merge_with(right_ms, extend_left);
                merging_segments[node->name] = merged_segment;

                // Update capacitance
                double wire_cap = delay_calculator->calculate_wire_capacitance(distance);
                node->capacitance = node->left->capacitance + node->right->capacitance + wire_cap;

                return merged_segment;
            };

            compute_segment(root);
            return merging_segments;
        }

        /**
         * @brief Embed the clock tree by selecting actual positions for internal nodes
         */
        std::shared_ptr<TreeNode> embed_tree(
            const std::shared_ptr<TreeNode>& merging_tree,
            const std::map<std::string, ManhattanArc<Interval<int>, Interval<int>>>&
                merging_segments) {
            std::function<void(const std::shared_ptr<TreeNode>&,
                               const ManhattanArc<Interval<int>, Interval<int>>*)>
                embed_node;
            embed_node = [&](const std::shared_ptr<TreeNode>& node,
                             const ManhattanArc<Interval<int>, Interval<int>>* parent_segment) {
                if (!node) return;

                auto it = merging_segments.find(node->name);
                if (it == merging_segments.end()) {
                    throw std::runtime_error("Merging segment not found for node: " + node->name);
                }
                const auto& node_segment = it->second;

                if (!parent_segment) {
                    // Root node: choose upper corner
                    node->position = node_segment.get_upper_corner();
                } else {
                    // Internal node: find nearest point to parent
                    node->position = node_segment.nearest_point_to(node->parent->position);
                    if (node->parent) {
                        node->wire_length = node->position.min_dist_with(node->parent->position);
                    }
                }

                // Recursively embed children
                embed_node(node->left, &node_segment);
                embed_node(node->right, &node_segment);
            };

            embed_node(merging_tree, nullptr);
            return merging_tree;
        }

        /**
         * @brief Compute delays and other parameters for the entire tree
         */
        void compute_tree_parameters(const std::shared_ptr<TreeNode>& root) {
            std::function<void(const std::shared_ptr<TreeNode>&, double)> compute_delays;
            compute_delays = [&](const std::shared_ptr<TreeNode>& node, double parent_delay) {
                if (!node) return;

                if (node->parent) {
                    double wire_delay = delay_calculator->calculate_wire_delay(node->wire_length,
                                                                               node->capacitance);
                    node->delay = parent_delay + wire_delay;
                } else {
                    node->delay = 0.0;  // Root has zero delay
                }

                compute_delays(node->left, node->delay);
                compute_delays(node->right, node->delay);
            };

            compute_delays(root, 0.0);
        }

      public:
        /**
         * @brief Analyze clock skew in the constructed tree
         */
        SkewAnalysis analyze_skew(const std::shared_ptr<TreeNode>& root) {
            std::vector<double> sink_delays;

            std::function<void(const std::shared_ptr<TreeNode>&)> collect_sink_delays;
            collect_sink_delays = [&](const std::shared_ptr<TreeNode>& node) {
                if (!node) return;

                if (node->is_leaf()) {
                    sink_delays.push_back(node->delay);
                }

                collect_sink_delays(node->left);
                collect_sink_delays(node->right);
            };

            collect_sink_delays(root);

            if (sink_delays.empty()) {
                throw std::runtime_error("No sink delays collected");
            }

            double max_delay = *std::max_element(sink_delays.begin(), sink_delays.end());
            double min_delay = *std::min_element(sink_delays.begin(), sink_delays.end());
            double skew = max_delay - min_delay;

            return {max_delay,
                    min_delay,
                    skew,
                    sink_delays,
                    total_wirelength(root),
                    typeid(*delay_calculator).name()};
        }

      private:
        /**
         * @brief Compute total wirelength of the clock tree
         */
        int total_wirelength(const std::shared_ptr<TreeNode>& root) {
            int total = 0;

            std::function<void(const std::shared_ptr<TreeNode>&)> sum_wirelength;
            sum_wirelength = [&](const std::shared_ptr<TreeNode>& node) {
                if (!node) return;
                total += node->wire_length;
                sum_wirelength(node->left);
                sum_wirelength(node->right);
            };

            sum_wirelength(root);
            return total;
        }
    };

    /**
     * @brief Extract comprehensive statistics from the clock tree
     */
    inline TreeStatistics get_tree_statistics(const std::shared_ptr<TreeNode>& root) {
        TreeStatistics stats;

        std::function<void(const std::shared_ptr<TreeNode>&, const std::shared_ptr<TreeNode>&)>
            traverse;
        traverse
            = [&](const std::shared_ptr<TreeNode>& node, const std::shared_ptr<TreeNode>& parent) {
                  if (!node) return;

                  // Add node info
                  stats.nodes.push_back({node->name,
                                         {node->position.xcoord(), node->position.ycoord()},
                                         node->is_leaf() ? "sink" : "internal",
                                         node->delay,
                                         node->capacitance});

                  if (node->is_leaf()) {
                      stats.sinks.push_back(node->name);
                  }

                  // Add wire info if there's a parent
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

// Example usage function
inline void example_dme_usage() {
    using namespace recti;

    std::cout << "=== Linear Delay Model ===" << std::endl;

    // Create clock sinks
    std::vector<Sink> sinks
        = {Sink("s1", Point<int>(10, 20), 1.0), Sink("s2", Point<int>(30, 40), 1.0),
           Sink("s3", Point<int>(50, 10), 1.0), Sink("s4", Point<int>(70, 30), 1.0),
           Sink("s5", Point<int>(90, 50), 1.0)};

    auto linear_calc = std::make_unique<LinearDelayCalculator>(0.5, 0.2);
    DMEAlgorithm dme_linear(sinks, std::move(linear_calc));
    auto clock_tree_linear = dme_linear.build_clock_tree();
    auto analysis_linear = dme_linear.analyze_skew(clock_tree_linear);

    std::cout << "Delay Model: " << analysis_linear.delay_model << std::endl;
    std::cout << "Maximum delay: " << analysis_linear.max_delay << std::endl;
    std::cout << "Minimum delay: " << analysis_linear.min_delay << std::endl;
    std::cout << "Clock skew: " << analysis_linear.skew << std::endl;
    std::cout << "Total wirelength: " << analysis_linear.total_wirelength << std::endl;

    std::cout << "\n=== Elmore Delay Model ===" << std::endl;

    auto elmore_calc = std::make_unique<ElmoreDelayCalculator>(0.1, 0.2);
    DMEAlgorithm dme_elmore(sinks, std::move(elmore_calc));
    auto clock_tree_elmore = dme_elmore.build_clock_tree();
    auto analysis_elmore = dme_elmore.analyze_skew(clock_tree_elmore);

    std::cout << "Delay Model: " << analysis_elmore.delay_model << std::endl;
    std::cout << "Maximum delay: " << analysis_elmore.max_delay << std::endl;
    std::cout << "Minimum delay: " << analysis_elmore.min_delay << std::endl;
    std::cout << "Clock skew: " << analysis_elmore.skew << std::endl;
    std::cout << "Total wirelength: " << analysis_elmore.total_wirelength << std::endl;
}

// Main function for testing
// int main() {
//     try {
//         example_dme_usage();
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }
//     return 0;
// }
