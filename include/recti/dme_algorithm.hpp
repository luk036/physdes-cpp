#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "manhattan_arc.hpp"
#include "point.hpp"

namespace recti {

    /**
     * @brief Represents a clock sink with position and capacitance.
     */
    class Sink {
      public:
        /**
         * @brief Construct a Sink with name, position, and capacitance.
         *
         * @param name The name of the sink.
         * @param position The position of the sink (Point).
         * @param capacitance The capacitance of the sink (default: 1.0).
         */
        Sink(const std::string& name, const Point<int, int>& position, float capacitance = 1.0f)
            : name_(name), position_(position), capacitance_(capacitance) {}

        /**
         * @brief Get the name of the sink.
         */
        auto name() const -> const std::string& { return name_; }

        /**
         * @brief Get the position of the sink.
         */
        auto position() const -> const Point<int, int>& { return position_; }

        /**
         * @brief Get the capacitance of the sink.
         */
        auto capacitance() const -> float { return capacitance_; }

        /**
         * @brief Stream insertion operator for Sink.
         */
        template <class Stream> friend auto operator<<(Stream& out, const Sink& sink) -> Stream& {
            out << "Sink(name=" << sink.name_ << ", position=" << sink.position_
                << ", capacitance=" << sink.capacitance_ << ")";
            return out;
        }

      private:
        std::string name_;
        Point<int, int> position_;
        float capacitance_;
    };

    /**
     * @brief Represents a node in the clock tree.
     */
    class TreeNode {
      public:
        /**
         * @brief Construct a TreeNode with name and position.
         *
         * @param name The name of the node.
         * @param position The position of the node (Point).
         * @param capacitance The capacitance of the node (default: 0.0).
         */
        TreeNode(const std::string& name, const Point<int, int>& position, float capacitance = 0.0f)
            : name_(name), position_(position), capacitance_(capacitance) {}

        /**
         * @brief Get the name of the node.
         */
        auto name() const -> const std::string& { return name_; }

        /**
         * @brief Get the position of the node.
         */
        auto position() const -> const Point<int, int>& { return position_; }

        /**
         * @brief Set the position of the node.
         */
        auto set_position(const Point<int, int>& pos) -> void { position_ = pos; }

        /**
         * @brief Get the left child of the node.
         */
        auto left() const -> std::optional<TreeNode*> { return left_; }

        /**
         * @brief Get the right child of the node.
         */
        auto right() const -> std::optional<TreeNode*> { return right_; }

        /**
         * @brief Get the parent of the node.
         */
        auto parent() const -> std::optional<TreeNode*> { return parent_; }

        /**
         * @brief Set the left child of the node.
         */
        auto set_left(TreeNode* node) -> void { left_ = node; }

        /**
         * @brief Set the right child of the node.
         */
        auto set_right(TreeNode* node) -> void { right_ = node; }

        /**
         * @brief Set the parent of the node.
         */
        auto set_parent(TreeNode* node) -> void { parent_ = node; }

        /**
         * @brief Get the wire length of the node.
         */
        auto wire_length() const -> int { return wire_length_; }

        /**
         * @brief Set the wire length of the node.
         */
        auto set_wire_length(int length) -> void { wire_length_ = length; }

        /**
         * @brief Get the delay of the node.
         */
        auto delay() const -> float { return delay_; }

        /**
         * @brief Set the delay of the node.
         */
        auto set_delay(float delay) -> void { delay_ = delay; }

        /**
         * @brief Get the capacitance of the node.
         */
        auto capacitance() const -> float { return capacitance_; }

        /**
         * @brief Set the capacitance of the node.
         */
        auto set_capacitance(float cap) -> void { capacitance_ = cap; }

        /**
         * @brief Check if the node needs elongation.
         */
        auto need_elongation() const -> bool { return need_elongation_; }

        /**
         * @brief Set the elongation flag of the node.
         */
        auto set_need_elongation(bool value) -> void { need_elongation_ = value; }

        /**
         * @brief Stream insertion operator for TreeNode.
         */
        template <class Stream> friend auto operator<<(Stream& out, const TreeNode& node)
            -> Stream& {
            out << "TreeNode(name=" << node.name_ << ", position=" << node.position_ << ")";
            return out;
        }

      private:
        std::string name_;
        Point<int, int> position_;
        std::optional<TreeNode*> left_ = std::nullopt;
        std::optional<TreeNode*> right_ = std::nullopt;
        std::optional<TreeNode*> parent_ = std::nullopt;
        int wire_length_ = 0;
        float delay_ = 0.0f;
        float capacitance_ = 0.0f;
        bool need_elongation_ = false;
    };

    /**
     * @brief Abstract base class for delay calculation strategies.
     */
    class DelayCalculator {
      public:
        virtual ~DelayCalculator() = default;

        /**
         * @brief Calculate wire delay for given length and load capacitance.
         */
        virtual auto calculate_wire_delay(int length, float load_capacitance) const -> float = 0;

        /**
         * @brief Calculate delay per unit length for given load capacitance.
         */
        virtual auto calculate_wire_delay_per_unit(float load_capacitance) const -> float = 0;

        /**
         * @brief Calculate wire capacitance for given length.
         */
        virtual auto calculate_wire_capacitance(int length) const -> float = 0;

        /**
         * @brief Calculate extra length based on skew.
         */
        virtual auto calculate_tapping_point(TreeNode* node_left, TreeNode* node_right,
                                             int distance) const -> std::pair<int, float>
            = 0;
    };

    /**
     * @brief Linear delay model: delay = k * length.
     */
    class LinearDelayCalculator : public DelayCalculator {
      public:
        /**
         * @brief Construct a LinearDelayCalculator.
         *
         * @param delay_per_unit Delay per unit wire length.
         * @param capacitance_per_unit Capacitance per unit wire length.
         */
        LinearDelayCalculator(float delay_per_unit = 1.0f, float capacitance_per_unit = 1.0f)
            : delay_per_unit_(delay_per_unit), capacitance_per_unit_(capacitance_per_unit) {}

        auto calculate_wire_delay(int length, float /*load_capacitance*/) const -> float override {
            return delay_per_unit_ * static_cast<float>(length);
        }

        auto calculate_wire_delay_per_unit(float /*load_capacitance*/) const -> float override {
            return delay_per_unit_;
        }

        auto calculate_wire_capacitance(int length) const -> float override {
            return capacitance_per_unit_ * static_cast<float>(length);
        }

        auto calculate_tapping_point(TreeNode* node_left, TreeNode* node_right, int distance) const
            -> std::pair<int, float> override {
            float skew = node_right->delay() - node_left->delay();
            int extend_left = static_cast<int>(
                std::round((skew / delay_per_unit_ + static_cast<float>(distance)) / 2.0f));
            float delay_left
                = node_left->delay() + static_cast<float>(extend_left) * delay_per_unit_;
            node_left->set_wire_length(extend_left);
            node_right->set_wire_length(distance - extend_left);

            if (extend_left < 0) {
                // ic(extend_left); // Icecream debugging not supported in C++, commented out
                node_left->set_wire_length(0);
                node_right->set_wire_length(distance - extend_left);
                extend_left = 0;
                delay_left = node_left->delay();
                node_right->set_need_elongation(true);
            } else if (extend_left > distance) {
                // ic(extend_left);
                node_right->set_wire_length(0);
                node_right->set_wire_length(extend_left);
                extend_left = distance;
                delay_left = node_right->delay();
                node_left->set_need_elongation(true);
            }
            return {extend_left, delay_left};
        }

        /**
         * @brief Get delay per unit.
         */
        auto delay_per_unit() const -> float { return delay_per_unit_; }

      private:
        float delay_per_unit_;
        float capacitance_per_unit_;
    };

    /**
     * @brief Elmore delay model for RC trees.
     */
    class ElmoreDelayCalculator : public DelayCalculator {
      public:
        /**
         * @brief Construct an ElmoreDelayCalculator.
         *
         * @param unit_resistance Resistance per unit length.
         * @param unit_capacitance Capacitance per unit length.
         */
        ElmoreDelayCalculator(float unit_resistance = 1.0f, float unit_capacitance = 1.0f)
            : unit_resistance_(unit_resistance), unit_capacitance_(unit_capacitance) {}

        auto calculate_wire_delay(int length, float load_capacitance) const -> float override {
            float wire_resistance = unit_resistance_ * static_cast<float>(length);
            float wire_capacitance = unit_capacitance_ * static_cast<float>(length);
            return wire_resistance * (wire_capacitance / 2.0f + load_capacitance);
        }

        auto calculate_wire_delay_per_unit(float load_capacitance) const -> float override {
            return unit_resistance_ * (unit_capacitance_ / 2.0f + load_capacitance);
        }

        auto calculate_wire_capacitance(int length) const -> float override {
            return unit_capacitance_ * static_cast<float>(length);
        }

        auto calculate_tapping_point(TreeNode* node_left, TreeNode* node_right, int distance) const
            -> std::pair<int, float> override {
            float skew = node_right->delay() - node_left->delay();
            float r = static_cast<float>(distance) * unit_resistance_;
            float c = static_cast<float>(distance) * unit_capacitance_;
            float z = (skew + r * (node_right->capacitance() + c / 2.0f))
                      / (r * (c + node_right->capacitance() + node_left->capacitance()));
            int extend_left = static_cast<int>(std::round(z * static_cast<float>(distance)));
            float r_left = static_cast<float>(extend_left) * unit_resistance_;
            float c_left = static_cast<float>(extend_left) * unit_capacitance_;
            float delay_left
                = node_left->delay() + r_left * (c_left / 2.0f + node_left->capacitance());
            node_left->set_wire_length(extend_left);
            node_right->set_wire_length(distance - extend_left);

            if (extend_left < 0) {
                // ic(extend_left);
                node_left->set_wire_length(0);
                node_right->set_wire_length(distance - extend_left);
                extend_left = 0;
                delay_left = node_left->delay();
                node_right->set_need_elongation(true);
            } else if (extend_left > distance) {
                // ic(extend_left);
                node_right->set_wire_length(0);
                node_right->set_wire_length(extend_left);
                extend_left = distance;
                delay_left = node_right->delay();
                node_left->set_need_elongation(true);
            }
            return {extend_left, delay_left};
        }

        /**
         * @brief Get unit resistance.
         */
        auto unit_resistance() const -> float { return unit_resistance_; }

      private:
        float unit_resistance_;
        float unit_capacitance_;
    };

    /**
     * @brief Deferred Merge Embedding (DME) Algorithm for Clock Tree Synthesis.
     */
    class DMEAlgorithm {
      public:
        /**
         * @brief Construct a DMEAlgorithm.
         *
         * @param sinks List of clock sinks.
         * @param delay_calculator Strategy for delay calculation.
         */
        DMEAlgorithm(const std::vector<Sink>& sinks,
                     std::unique_ptr<DelayCalculator> delay_calculator)
            : sinks_(sinks), delay_calculator_(std::move(delay_calculator)), node_id_(0) {
            if (sinks.empty()) {
                throw std::invalid_argument("No sinks provided");
            }
        }

        /**
         * @brief Build a zero-skew clock tree for the given sinks.
         */
        auto build_clock_tree() -> std::unique_ptr<TreeNode> {
            // Step 1: Create initial leaf nodes
            std::vector<std::unique_ptr<TreeNode>> nodes;
            for (const auto& sink : sinks_) {
                nodes.emplace_back(
                    std::make_unique<TreeNode>(sink.name(), sink.position(), sink.capacitance()));
            }

            // Step 2: Build merging tree
            auto merging_tree = build_merging_tree(std::move(nodes), false);

            // Step 3: Compute merging segments
            auto merging_segments = compute_merging_segments(merging_tree.get());

            // Step 4: Embed tree
            auto clock_tree = embed_tree(merging_tree.get(), merging_segments);

            // Step 5: Compute delays and wire lengths
            compute_tree_parameters(clock_tree.get());

            return clock_tree;
        }

        /**
         * @brief Analyze clock skew in the constructed tree.
         */
        auto analyze_skew(const TreeNode* root) const
            -> std::unordered_map<std::string,
                                  std::variant<float, std::string, std::vector<float>>> {
            std::vector<float> sink_delays;

            std::function<void(const TreeNode*)> collect_sink_delays = [&](const TreeNode* node) {
                if (!node) return;
                if (!node->left() && !node->right()) {
                    sink_delays.push_back(node->delay());
                }
                if (node->left()) collect_sink_delays(node->left().value());
                if (node->right()) collect_sink_delays(node->right().value());
            };

            collect_sink_delays(root);

            float max_delay = *std::max_element(sink_delays.begin(), sink_delays.end());
            float min_delay = *std::min_element(sink_delays.begin(), sink_delays.end());
            float skew = max_delay - min_delay;

            return {{"max_delay", max_delay},
                    {"min_delay", min_delay},
                    {"skew", skew},
                    {"sink_delays", sink_delays},
                    {"total_wirelength", static_cast<float>(total_wirelength(root))},
                    {"delay_model", std::string("TBD")}};
        }

      private:
        /**
         * @brief Build a balanced merging tree using recursive bipartition.
         */
        auto build_merging_tree(std::vector<std::unique_ptr<TreeNode>> nodes, bool vertical)
            -> std::unique_ptr<TreeNode> {
            if (nodes.size() == 1) {
                return std::move(nodes[0]);
            }

            // Sort nodes by x or y coordinate
            std::sort(nodes.begin(), nodes.end(), [vertical](const auto& a, const auto& b) {
                return vertical ? a->position().xcoord() < b->position().xcoord()
                                : a->position().ycoord() < b->position().ycoord();
            });

            // Split into left and right groups
            size_t mid = nodes.size() / 2;
            std::vector<std::unique_ptr<TreeNode>> left_group(
                std::make_move_iterator(nodes.begin()),
                std::make_move_iterator(nodes.begin() + mid));
            std::vector<std::unique_ptr<TreeNode>> right_group(
                std::make_move_iterator(nodes.begin() + mid), std::make_move_iterator(nodes.end()));

            // Recursively build subtrees
            auto left_child = build_merging_tree(std::move(left_group), !vertical);
            auto right_child = build_merging_tree(std::move(right_group), !vertical);

            // Create parent node
            auto parent = std::make_unique<TreeNode>("n" + std::to_string(node_id_++),
                                                     left_child->position());
            parent->set_left(left_child.get());
            parent->set_right(right_child.get());
            left_child->set_parent(parent.get());
            right_child->set_parent(parent.get());

            return parent;
        }

        /**
         * @brief Compute merging segments for all nodes in bottom-up order.
         */
        auto compute_merging_segments(TreeNode* root)
            -> std::unordered_map<std::string, ManhattanArc<Interval<int>, Interval<int>>> {
            std::unordered_map<std::string, ManhattanArc<Interval<int>, Interval<int>>>
                merging_segments;

            std::function<ManhattanArc<Interval<int>, Interval<int>>(TreeNode*)> compute_segment
                = [&](TreeNode* node) {
                      if (!node->left() && !node->right()) {
                          auto ms1 = ManhattanArc<int, int>::from_point(node->position());
                          ManhattanArc<Interval<int>, Interval<int>> ms{
                              Interval{ms1.impl.xcoord(), ms1.impl.xcoord()},
                              Interval{ms1.impl.ycoord(), ms1.impl.ycoord()}};
                          merging_segments.emplace(node->name(), ms);
                          return ms;
                      }

                      if (!node->left() || !node->right()) {
                          throw std::runtime_error("Internal node must have both children");
                      }

                      auto left_ms = compute_segment(node->left().value());
                      auto right_ms = compute_segment(node->right().value());
                      int distance = left_ms.min_dist_with(right_ms);

                      auto [extend_left, delay_left] = delay_calculator_->calculate_tapping_point(
                          node->left().value(), node->right().value(), distance);
                      node->set_delay(delay_left);

                      auto merged_segment = left_ms.merge_with(right_ms, extend_left);
                      merging_segments.emplace(node->name(), merged_segment);

                      float wire_cap = delay_calculator_->calculate_wire_capacitance(distance);
                      node->set_capacitance(node->left().value()->capacitance()
                                            + node->right().value()->capacitance() + wire_cap);

                      return merged_segment;
                  };

            compute_segment(root);
            return merging_segments;
        }

        /**
         * @brief Embed the clock tree by selecting actual positions for internal nodes.
         */
        auto embed_tree(
            TreeNode* merging_tree,
            const std::unordered_map<std::string, ManhattanArc<Interval<int>, Interval<int>>>&
                merging_segments) -> std::unique_ptr<TreeNode> {
            std::function<void(TreeNode*,
                               const std::optional<ManhattanArc<Interval<int>, Interval<int>>>)>
                embed_node = [&](TreeNode* node,
                                 const std::optional<ManhattanArc<Interval<int>, Interval<int>>>
                                     parent_segment) {
                    if (!node) return;

                    if (!parent_segment) {
                        node->set_position(merging_segments.at(node->name()).get_upper_corner());
                    } else {
                        node->set_position(
                            merging_segments.at(node->name())
                                .nearest_point_to(node->parent().value()->position()));
                        node->set_wire_length(
                            node->position().min_dist_with(node->parent().value()->position()));
                    }

                    embed_node(node->left().value_or(nullptr), merging_segments.at(node->name()));
                    embed_node(node->right().value_or(nullptr), merging_segments.at(node->name()));
                };

            auto tree = std::make_unique<TreeNode>(*merging_tree);
            embed_node(tree.get(), std::nullopt);
            return tree;
        }

        /**
         * @brief Compute delays and other parameters for the entire tree.
         */
        auto compute_tree_parameters(TreeNode* root) -> void {
            std::function<void(TreeNode*, float)> compute_delays
                = [&](TreeNode* node, float parent_delay) {
                      if (!node) return;

                      if (node->parent()) {
                          float wire_delay = delay_calculator_->calculate_wire_delay(
                              node->wire_length(), node->capacitance());
                          node->set_delay(parent_delay + wire_delay);
                      } else {
                          node->set_delay(0.0f);
                      }

                      compute_delays(node->left().value_or(nullptr), node->delay());
                      compute_delays(node->right().value_or(nullptr), node->delay());
                  };

            compute_delays(root, 0.0f);
        }

        /**
         * @brief Compute total wirelength of the clock tree.
         */
        auto total_wirelength(const TreeNode* root) const -> int {
            int total = 0;
            std::function<void(const TreeNode*)> sum_wirelength = [&](const TreeNode* node) {
                if (!node) return;
                total += node->wire_length();
                if (node->left()) sum_wirelength(node->left().value());
                if (node->right()) sum_wirelength(node->right().value());
            };
            sum_wirelength(root);
            return total;
        }

        std::vector<Sink> sinks_;
        std::unique_ptr<DelayCalculator> delay_calculator_;
        int node_id_;
    };

    /**
     * @brief Extract comprehensive statistics from the clock tree.
     */
    inline auto get_tree_statistics(const TreeNode* root) -> std::unordered_map<
        std::string,
        std::variant<std::vector<std::unordered_map<
                         std::string, std::variant<std::string, std::tuple<int, int>, float, int>>>,
                     std::vector<std::string>, int>> {
        using Values = std::variant<std::string, std::tuple<int, int>, float, int>;
        std::vector<std::unordered_map<std::string, Values>> nodes;
        std::vector<std::unordered_map<std::string, Values>> wires;
        std::vector<std::string> sinks;

        std::function<void(const TreeNode*, const TreeNode*)> traverse =
            [&](const TreeNode* node, const TreeNode* parent) {
                if (!node) {
                    return;
                }

                nodes.push_back(
                    {{"name", node->name()},
                     {"position", std::tuple(node->position().xcoord(), node->position().ycoord())},
                     {"type", node->left() || node->right() ? std::string("internal")
                                                            : std::string("sink")},
                     {"delay", node->delay()},
                     {"capacitance", node->capacitance()}});

                if (!node->left() && !node->right()) {
                    sinks.push_back(node->name());
                }

                if (parent) {
                    wires.push_back({{"from", parent->name()},
                                     {"to", node->name()},
                                     {"length", node->wire_length()},
                                     {"from_pos", std::tuple(parent->position().xcoord(),
                                                             parent->position().ycoord())},
                                     {"to_pos", std::tuple(node->position().xcoord(),
                                                           node->position().ycoord())}});
                }

                traverse(node->left().value_or(nullptr), node);
                traverse(node->right().value_or(nullptr), node);
            };

        traverse(root, nullptr);

        using StatsVariant = std::variant<std::vector<std::unordered_map<std::string, Values>>,
                                          std::vector<std::string>, int>;

        return {{"nodes", StatsVariant(nodes)},
                {"wires", StatsVariant(wires)},
                {"sinks", StatsVariant(sinks)},
                {"total_nodes", static_cast<int>(nodes.size())},
                {"total_sinks", static_cast<int>(sinks.size())},
                {"total_wires", static_cast<int>(wires.size())}};
    }

}  // namespace recti
