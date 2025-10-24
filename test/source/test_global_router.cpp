#include <doctest/doctest.h>
#include <fmt/core.h>

#include <iostream>         // for operator<<
#include <ldsgen/ilds.hpp>  // for VdCorput
#include <recti/global_router.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <string>
#include <vector>

using namespace recti;

TEST_SUITE("RoutingNode") {
    TEST_CASE("RoutingNode::remove_child - existing child") {
        RoutingNode parent("parent", NodeType::SOURCE, Point{0, 0});
        RoutingNode child1("child1", NodeType::TERMINAL, Point{0, 0});
        RoutingNode child2("child2", NodeType::TERMINAL, Point{0, 0});

        parent.add_child(&child1);
        parent.add_child(&child2);

        CHECK(parent.children.size() == 2);
        CHECK(child1.parent == &parent);
        CHECK(child2.parent == &parent);

        parent.remove_child(&child1);

        CHECK(parent.children.size() == 1);
        CHECK(parent.children[0] == &child2);
        CHECK(child1.parent == nullptr);
    }

    TEST_CASE("RoutingNode::remove_child - non-existent child") {
        RoutingNode parent("parent", NodeType::SOURCE, Point{0, 0});
        RoutingNode child1("child1", NodeType::TERMINAL, Point{0, 0});
        RoutingNode child2("child2", NodeType::TERMINAL, Point{0, 0});

        parent.add_child(&child1);

        CHECK(parent.children.size() == 1);
        CHECK(child1.parent == &parent);

        parent.remove_child(&child2);  // Try to remove a child that was never added

        CHECK(parent.children.size() == 1);  // Size should remain 1
        CHECK(parent.children[0] == &child1);
        CHECK(child1.parent == &parent);
        CHECK(child2.parent == nullptr);  // Should still be nullptr
    }

    TEST_CASE("RoutingNode::remove_child - from empty children list") {
        RoutingNode parent("parent", NodeType::SOURCE, Point{0, 0});
        RoutingNode child("child", NodeType::TERMINAL, Point{0, 0});

        CHECK(parent.children.empty());

        parent.remove_child(&child);  // Remove from empty list

        CHECK(parent.children.empty());
        CHECK(child.parent == nullptr);
    }
}

TEST_SUITE("GlobalRoutingTree") {
    TEST_CASE("GlobalRoutingTree::insert_steiner_node - invalid parent_id") {
        GlobalRoutingTree tree(Point{0, 0});
        CHECK_THROWS_AS(tree.insert_steiner_node({10, 10}, "non_existent_parent"),
                        std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::insert_terminal_node - invalid parent_id") {
        GlobalRoutingTree tree(Point{0, 0});
        CHECK_THROWS_AS(tree.insert_terminal_node({10, 10}, "non_existent_parent"),
                        std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::insert_node_on_branch - basic insertion") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0}, s1_id);

        CHECK(tree.nodes.at(s1_id)->children.size() == 1);
        CHECK(tree.nodes.at(s1_id)->children[0]->id == s2_id);

        std::string new_s_id
            = tree.insert_node_on_branch(NodeType::STEINER, Point{15, 0}, s1_id, s2_id);

        CHECK(tree.nodes.at(s1_id)->children.size() == 1);
        CHECK(tree.nodes.at(s1_id)->children[0]->id == new_s_id);
        CHECK(tree.nodes.at(new_s_id)->children.size() == 1);
        CHECK(tree.nodes.at(new_s_id)->children[0]->id == s2_id);
        CHECK(tree.nodes.at(s2_id)->parent->id == new_s_id);
    }

    TEST_CASE("GlobalRoutingTree::insert_node_on_branch - invalid branch_start_id") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0}, s1_id);
        CHECK_THROWS_AS(
            tree.insert_node_on_branch(NodeType::STEINER, Point{15, 0}, "non_existent", s2_id),
            std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::insert_node_on_branch - invalid branch_end_id") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0}, s1_id);
        CHECK_THROWS_AS(
            tree.insert_node_on_branch(NodeType::STEINER, Point{15, 0}, s1_id, "non_existent"),
            std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::insert_node_on_branch - end_node not child of start_node") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0});  // s2 is not child of s1

        CHECK_THROWS_AS(tree.insert_node_on_branch(NodeType::STEINER, Point{15, 0}, s1_id, s2_id),
                        std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::find_path_to_source - basic path") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0}, s1_id);
        std::string t1_id = tree.insert_terminal_node({30, 0}, s2_id);

        auto path = tree.find_path_to_source(t1_id);
        CHECK(path.size() == 4);  // source -> s1 -> s2 -> t1
        CHECK(path[0]->id == "source");
        CHECK(path[1]->id == s1_id);
        CHECK(path[2]->id == s2_id);
        CHECK(path[3]->id == t1_id);
    }

    TEST_CASE("GlobalRoutingTree::find_path_to_source - source node") {
        GlobalRoutingTree tree(Point{0, 0});
        auto path = tree.find_path_to_source("source");
        CHECK(path.size() == 1);
        CHECK(path[0]->id == "source");
    }

    TEST_CASE("GlobalRoutingTree::find_path_to_source - non-existent node") {
        GlobalRoutingTree tree(Point{0, 0});
        CHECK_THROWS_AS(tree.find_path_to_source("non_existent"), std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::insert_terminal_with_steiner - no steiner inserted") {
        GlobalRoutingTree tree(Point{0, 0});
        tree.insert_terminal_with_steiner({0, 10});  // Should attach directly to source

        CHECK(tree.get_source()->children.size() == 1);
        CHECK(tree.get_all_steiner_nodes().empty());
        CHECK(tree.get_all_terminals().size() == 1);
    }

    TEST_CASE("GlobalRoutingTree::insert_terminal_with_steiner - steiner inserted on branch") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string t1_id = tree.insert_terminal_node({20, 0}, s1_id);

        // Insert a terminal that should cause a steiner to be inserted on the source-s1 branch
        tree.insert_terminal_with_steiner({5, 5});

        CHECK(tree.get_all_steiner_nodes().size() == 2);  // s1 and the new steiner
        CHECK(tree.get_all_terminals().size() == 2);      // t1 and the new terminal

        // Verify the new steiner is on the source-s1 branch
        RoutingNode<Point<int, int>>* source = tree.get_source();
        CHECK(source->children.size() == 1);
        RoutingNode<Point<int, int>>* new_steiner = source->children[0];
        CHECK(new_steiner->type == NodeType::STEINER);
        CHECK(new_steiner->pt.xcoord() == 5);  // Nearest point on source-s1 path to (5,5) is (5,0)
        CHECK(new_steiner->pt.ycoord() == 0);

        CHECK(new_steiner->children.size()
              == 2);  // Should have s1 and the new terminal as children
        bool found_s1 = false;
        bool found_new_terminal = false;
        for (auto child : new_steiner->children) {
            if (child->id == s1_id) found_s1 = true;
            if (child->type == NodeType::TERMINAL && child->pt.xcoord() == 5
                && child->pt.ycoord() == 5)
                found_new_terminal = true;
        }
        CHECK(found_s1);
        CHECK(found_new_terminal);
    }
}

// Helper function to generate a set of points for testing
auto generate_points(size_t num_terminals, unsigned int seed)
    -> std::pair<Point<int, int>, std::vector<Point<int, int>>> {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(seed);
    hgenY.reseed(seed);

    using IntPoint = Point<int, int>;

    std::vector<IntPoint> terminals;
    terminals.reserve(num_terminals);
    for (size_t i = 0; i < num_terminals; ++i) {
        terminals.emplace_back(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop()));
    }
    IntPoint source(static_cast<int>(hgenX.pop()), static_cast<int>(hgenX.pop()));

    return {source, terminals};
}

TEST_SUITE("RoutingAlgorithms") {
    TEST_CASE("Test routing algorithms") {
        const size_t num_terminals = 7;
        const unsigned int seed = 19;
        const double high_stress = 1.0;
        const unsigned int width = 1000;
        const unsigned int height = 1000;

        auto [source, terminals] = generate_points(num_terminals, seed);

        SUBCASE("route_with_steiners") {
            GlobalRouter router(source, terminals);
            router.route_with_steiners();
            save_routing_tree_svg(router.get_tree(), std::nullopt,
                                  "example_route_with_steiner.svg", width, height);
        }

        SUBCASE("route_with_constraints") {
            GlobalRouter router(source, terminals);
            router.route_with_constraints(high_stress);
            save_routing_tree_svg(router.get_tree(), std::nullopt,
                                  "example_route_with_constraint.svg", width, height);
        }
    }
}
