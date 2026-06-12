#include <fmt/core.h>

#include <algorithm>
#include <iostream>
#include <recti/dme_algorithm.hpp>
#include <recti/logger.hpp>

namespace recti {

    // ---------------------------------------------------------------------------
    // LinearDelayCalculator
    // ---------------------------------------------------------------------------
    TappingResult LinearDelayCalculator::calculate_tapping_point(
        int distance, double left_delay, double right_delay,
        double /* left_capacitance */, double /* right_capacitance */) const {
        if (distance == 0) {
            return {0, 0, std::max(left_delay, right_delay)};
        }
        double skew = right_delay - left_delay;
        int raw_extend_left
            = static_cast<int>(std::round((skew / delay_per_unit + distance) / 2));
        double delay_left = left_delay + raw_extend_left * delay_per_unit;

        int extend_left = raw_extend_left;
        if (raw_extend_left < 0) {
            extend_left = 0;
            delay_left = left_delay;
        } else if (raw_extend_left > distance) {
            extend_left = distance;
            delay_left = right_delay;
        }
        return {extend_left, raw_extend_left, delay_left};
    }

    // ---------------------------------------------------------------------------
    // ElmoreDelayCalculator
    // ---------------------------------------------------------------------------
    TappingResult ElmoreDelayCalculator::calculate_tapping_point(
        int distance, double left_delay, double right_delay,
        double left_capacitance, double right_capacitance) const {
        if (distance == 0) {
            return {0, 0, std::max(left_delay, right_delay)};
        }
        double skew = right_delay - left_delay;
        double r = distance * unit_resistance;
        double c = distance * unit_capacitance;

        double z = (skew + r * (right_capacitance + c / 2.0))
                   / (r * (c + right_capacitance + left_capacitance));

        int raw_extend_left = static_cast<int>(std::round(z * distance));
        double r_left = raw_extend_left * unit_resistance;
        double c_left = raw_extend_left * unit_capacitance;
        double delay_left
            = left_delay + r_left * (c_left / 2.0 + left_capacitance);

        int extend_left = raw_extend_left;
        if (raw_extend_left < 0) {
            extend_left = 0;
            delay_left = left_delay;
        } else if (raw_extend_left > distance) {
            extend_left = distance;
            delay_left = right_delay;
        }
        return {extend_left, raw_extend_left, delay_left};
    }

    // -----------------------------------------------------------------------
    // DMEAlgorithm
    // -----------------------------------------------------------------------
    NodeIdx DMEAlgorithm::build_clock_tree() {
        // Step 1: Create leaf nodes from sinks
        tree = Tree{};
        node_id = 0;

        for (const auto& sink : sinks) {
            TreeNode node(sink.name, sink.position);
            node.capacitance = sink.capacitance;
            tree.add(std::move(node));
        }

        // Step 2: Collect leaf indices
        std::vector<NodeIdx> leaf_indices;
        leaf_indices.reserve(tree.size());
        for (NodeIdx i = 0; i < tree.size(); ++i) {
            leaf_indices.push_back(i);
        }

        // Step 3: Build merging tree topology
        NodeIdx merging_root = build_merging_tree(leaf_indices, false);

        // Step 4: Bottom-up merging segment computation
        auto merging_segments = compute_merging_segments(merging_root);

        // Step 5: Top-down embedding
        NodeIdx clock_root = embed_tree(merging_root, merging_segments);

        // Step 6: Compute final delays
        compute_tree_parameters(clock_root);

        tree.root = clock_root;
        return clock_root;
    }

    NodeIdx DMEAlgorithm::build_merging_tree(const std::vector<NodeIdx>& node_ids, bool vertical) {
        if (node_ids.size() == 1) {
            return node_ids[0];
        }

        auto sorted = node_ids;
        if (vertical) {
            std::ranges::sort(sorted, [this](NodeIdx a, NodeIdx b) {
                return tree.get(a).position.xcoord() < tree.get(b).position.xcoord();
            });
        } else {
            std::ranges::sort(sorted, [this](NodeIdx a, NodeIdx b) {
                return tree.get(a).position.ycoord() < tree.get(b).position.ycoord();
            });
        }

        size_t mid = sorted.size() / 2;
        std::vector<NodeIdx> left_group(sorted.begin(),
                                        sorted.begin() + static_cast<std::ptrdiff_t>(mid));
        std::vector<NodeIdx> right_group(
            sorted.begin() + static_cast<std::ptrdiff_t>(mid), sorted.end());

        NodeIdx left_child = build_merging_tree(left_group, !vertical);
        NodeIdx right_child = build_merging_tree(right_group, !vertical);

        std::string id = fmt::format("n{}", node_id++);
        Point<int> pos = tree.get(left_child).position;
        NodeIdx parent_idx = tree.add(TreeNode(id, pos));

        tree.get_mut(parent_idx).left = left_child;
        tree.get_mut(parent_idx).right = right_child;
        tree.get_mut(left_child).parent = parent_idx;
        tree.get_mut(right_child).parent = parent_idx;

        return parent_idx;
    }

    ManhattanArc<Interval<int>, Interval<int>> DMEAlgorithm::_compute_merging_segment(
        NodeIdx node,
        std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>&
            merging_segments) {
        if (tree.get(node).is_leaf()) {
            Point<int> pos = tree.get(node).position;
            auto ms1 = ManhattanArc<int, int>::from_point(pos);
            ManhattanArc<Interval<int>, Interval<int>> ms(
                Interval{ms1.impl.xcoord(), ms1.impl.xcoord()},
                Interval{ms1.impl.ycoord(), ms1.impl.ycoord()});
            merging_segments[node] = ms;
            return ms;
        }

        NodeIdx left = tree.get(node).left;
        NodeIdx right = tree.get(node).right;
        if (left == SIZE_MAX || right == SIZE_MAX) {
            throw std::runtime_error("Internal node must have both left and right children");
        }

        auto left_ms = _compute_merging_segment(left, merging_segments);
        auto right_ms = _compute_merging_segment(right, merging_segments);
        int distance = left_ms.min_dist_with(right_ms);

        // Read child delays and capacitances
        double left_delay = tree.get(left).delay;
        double right_delay = tree.get(right).delay;
        double left_cap = tree.get(left).capacitance;
        double right_cap = tree.get(right).capacitance;

        TappingResult tp = delay_calculator->calculate_tapping_point(
            distance, left_delay, right_delay, left_cap, right_cap);

        // Apply node side-effects: wire_length and need_elongation
        {
            auto [l_node, r_node] = tree.get_pair_mut(left, right);
            l_node.wire_length = tp.extend_left;
            r_node.wire_length = distance - tp.raw_extend_left;

            if (tp.raw_extend_left < 0) {
                l_node.wire_length = 0;
                r_node.wire_length = distance - tp.raw_extend_left;
                r_node.need_elongation = true;
                log_with_spdlog(
                    "Warning: Right node needs elongation: extend_left < 0  => extend_left "
                    "set to 0");
            } else if (tp.raw_extend_left > distance) {
                r_node.wire_length = 0;
                l_node.wire_length = tp.raw_extend_left;
                l_node.need_elongation = true;
                log_with_spdlog(
                    "Warning: Left node needs elongation: extend_left > distance => "
                    "extend_left set to distance");
            }
        }

        tree.get_mut(node).delay = tp.delay_left;

        auto merged_segment = left_ms.merge_with(right_ms, tp.extend_left);
        merging_segments[node] = merged_segment;

        double wire_cap = delay_calculator->calculate_wire_capacitance(distance);
        tree.get_mut(node).capacitance
            = tree.get(left).capacitance + tree.get(right).capacitance + wire_cap;

        return merged_segment;
    }

    std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>
    DMEAlgorithm::compute_merging_segments(NodeIdx root) {
        std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>
            merging_segments;
        _compute_merging_segment(root, merging_segments);
        return merging_segments;
    }

    void DMEAlgorithm::_embed_node(
        NodeIdx node, const ManhattanArc<Interval<int>, Interval<int>>* parent_segment,
        const std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>&
            merging_segments) {
        if (node == SIZE_MAX) return;

        auto it = merging_segments.find(node);
        if (it == merging_segments.end()) {
            throw std::runtime_error("Merging segment not found for node: "
                                     + tree.get(node).name);
        }
        const auto& node_segment = it->second;

        if (parent_segment == nullptr) {
            // Root node: pick upper corner of its merging segment
            tree.get_mut(node).position = node_segment.get_upper_corner();
        } else {
            // Internal node: nearest point on segment to parent
            NodeIdx p = tree.get(node).parent;
            Point<int> parent_pos = tree.get(p).position;
            tree.get_mut(node).position = node_segment.nearest_point_to(parent_pos);
            tree.get_mut(node).wire_length
                = tree.get(node).position.min_dist_with(parent_pos);
        }

        _embed_node(tree.get(node).left, &node_segment, merging_segments);
        _embed_node(tree.get(node).right, &node_segment, merging_segments);
    }

    NodeIdx DMEAlgorithm::embed_tree(
        NodeIdx merging_tree_root,
        const std::unordered_map<NodeIdx, ManhattanArc<Interval<int>, Interval<int>>>&
            merging_segments) {
        _embed_node(merging_tree_root, nullptr, merging_segments);
        return merging_tree_root;
    }

    void DMEAlgorithm::_compute_delays(NodeIdx node, double parent_delay) {
        if (node == SIZE_MAX) return;

        if (tree.get(node).parent != SIZE_MAX) {
            double wire_delay = delay_calculator->calculate_wire_delay(
                tree.get(node).wire_length, tree.get(node).capacitance);
            tree.get_mut(node).delay = parent_delay + wire_delay;
        } else {
            tree.get_mut(node).delay = 0.0;
        }

        double current_delay = tree.get(node).delay;
        _compute_delays(tree.get(node).left, current_delay);
        _compute_delays(tree.get(node).right, current_delay);
    }

    void DMEAlgorithm::compute_tree_parameters(NodeIdx root) {
        _compute_delays(root, 0.0);
    }

    // -----------------------------------------------------------------------
    // Free helpers for analyze_skew / total_wirelength
    // -----------------------------------------------------------------------
    namespace {
        void collect_sink_delays(const Tree& tree, NodeIdx node,
                                 std::vector<double>& sink_delays) {
            if (node == SIZE_MAX) return;
            if (tree.get(node).is_leaf()) {
                sink_delays.push_back(tree.get(node).delay);
            }
            collect_sink_delays(tree, tree.get(node).left, sink_delays);
            collect_sink_delays(tree, tree.get(node).right, sink_delays);
        }

        int sum_wirelength(const Tree& tree, NodeIdx node) {
            if (node == SIZE_MAX) return 0;
            return tree.get(node).wire_length + sum_wirelength(tree, tree.get(node).left)
                   + sum_wirelength(tree, tree.get(node).right);
        }
    }  // anonymous namespace

    SkewAnalysis DMEAlgorithm::analyze_skew(NodeIdx root) const {
        std::vector<double> sink_delays;
        collect_sink_delays(tree, root, sink_delays);
        if (sink_delays.empty()) {
            throw std::runtime_error("No sink delays collected");
        }
        double max_delay = *std::ranges::max_element(sink_delays);
        double min_delay = *std::ranges::min_element(sink_delays);
        double skew = max_delay - min_delay;

        return {.max_delay = max_delay,
                .min_delay = min_delay,
                .skew = skew,
                .sink_delays = sink_delays,
                .total_wirelength = total_wirelength(root),
                .delay_model = typeid(*delay_calculator).name()};
    }

    int DMEAlgorithm::total_wirelength(NodeIdx root) const {
        return sum_wirelength(tree, root);
    }

    // -----------------------------------------------------------------------
    // get_tree_statistics
    // -----------------------------------------------------------------------
    TreeStatistics get_tree_statistics(const Tree& tree, NodeIdx root) {
        TreeStatistics stats;

        struct TraverseHelper {
            const Tree& tree;
            TreeStatistics& stats;

            void operator()(NodeIdx node, NodeIdx parent) {
                if (node == SIZE_MAX) return;
                const auto& n = tree.get(node);
                stats.nodes.push_back(
                    {.name = n.name,
                     .position = {n.position.xcoord(), n.position.ycoord()},
                     .type = n.is_leaf() ? "sink" : "internal",
                     .delay = n.delay,
                     .capacitance = n.capacitance});
                if (n.is_leaf()) {
                    stats.sinks.emplace_back(n.name);
                }
                if (parent != SIZE_MAX) {
                    const auto& p = tree.get(parent);
                    stats.wires.push_back(
                        {.from_node = p.name,
                         .to_node = n.name,
                         .length = n.wire_length,
                         .from_pos = {p.position.xcoord(), p.position.ycoord()},
                         .to_pos = {n.position.xcoord(), n.position.ycoord()}});
                }
                (*this)(n.left, node);
                (*this)(n.right, node);
            }
        };

        TraverseHelper{tree, stats}(root, SIZE_MAX);

        stats.total_nodes = static_cast<int>(stats.nodes.size());
        stats.total_sinks = static_cast<int>(stats.sinks.size());
        stats.total_wires = static_cast<int>(stats.wires.size());

        return stats;
    }

}  // namespace recti

// ---------------------------------------------------------------------------
// example_dme_usage
// ---------------------------------------------------------------------------
void example_dme_usage() {
    using namespace recti;

    std::cout << "=== Linear Delay Model ===" << '\n';

    std::vector<Sink> sinks
        = {Sink("s1", Point<int>(10, 20), 1.0), Sink("s2", Point<int>(30, 40), 1.0),
           Sink("s3", Point<int>(50, 10), 1.0), Sink("s4", Point<int>(70, 30), 1.0),
           Sink("s5", Point<int>(90, 50), 1.0)};

    // Linear model
    auto linear_calc = std::make_unique<LinearDelayCalculator>(0.5, 0.2);
    DMEAlgorithm dme_linear(sinks, std::move(linear_calc));
    NodeIdx root_linear = dme_linear.build_clock_tree();
    auto analysis_linear = dme_linear.analyze_skew(root_linear);

    std::cout << "Delay Model: " << analysis_linear.delay_model << '\n';
    std::cout << "Maximum delay: " << analysis_linear.max_delay << '\n';
    std::cout << "Minimum delay: " << analysis_linear.min_delay << '\n';
    std::cout << "Clock skew: " << analysis_linear.skew << '\n';
    std::cout << "Total wirelength: " << analysis_linear.total_wirelength << '\n';

    std::cout << "\n=== Elmore Delay Model ===\n";

    // Elmore model
    auto elmore_calc = std::make_unique<ElmoreDelayCalculator>(0.1, 0.2);
    DMEAlgorithm dme_elmore(sinks, std::move(elmore_calc));
    NodeIdx root_elmore = dme_elmore.build_clock_tree();
    auto analysis_elmore = dme_elmore.analyze_skew(root_elmore);

    std::cout << "Delay Model: " << analysis_elmore.delay_model << '\n';
    std::cout << "Maximum delay: " << analysis_elmore.max_delay << '\n';
    std::cout << "Minimum delay: " << analysis_elmore.min_delay << '\n';
    std::cout << "Clock skew: " << analysis_elmore.skew << '\n';
    std::cout << "Total wirelength: " << analysis_elmore.total_wirelength << '\n';
}
