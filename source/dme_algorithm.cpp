#include <recti/dme_algorithm.hpp>
#include <iostream>
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

// get_tree_statistics implementation
TreeStatistics get_tree_statistics(const std::shared_ptr<TreeNode>& root) {
    TreeStatistics stats;
    std::function<void(const std::shared_ptr<TreeNode>&, const std::shared_ptr<TreeNode>&)>
        traverse;

    traverse = [&](const std::shared_ptr<TreeNode>& node, const std::shared_ptr<TreeNode>& parent) {
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
