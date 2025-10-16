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
        RoutingNode parent("parent", NodeType::SOURCE);
        RoutingNode child1("child1", NodeType::TERMINAL);
        RoutingNode child2("child2", NodeType::TERMINAL);

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
        RoutingNode parent("parent", NodeType::SOURCE);
        RoutingNode child1("child1", NodeType::TERMINAL);
        RoutingNode child2("child2", NodeType::TERMINAL);

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
        RoutingNode parent("parent", NodeType::SOURCE);
        RoutingNode child("child", NodeType::TERMINAL);

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

        std::string new_s_id = tree.insert_node_on_branch(NodeType::STEINER, 15, 0, s1_id, s2_id);

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
        CHECK_THROWS_AS(tree.insert_node_on_branch(NodeType::STEINER, 15, 0, "non_existent", s2_id),
                        std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::insert_node_on_branch - invalid branch_end_id") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0}, s1_id);
        CHECK_THROWS_AS(tree.insert_node_on_branch(NodeType::STEINER, 15, 0, s1_id, "non_existent"),
                        std::runtime_error);
    }

    TEST_CASE("GlobalRoutingTree::insert_node_on_branch - end_node not child of start_node") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0});  // s2 is not child of s1

        CHECK_THROWS_AS(tree.insert_node_on_branch(NodeType::STEINER, 15, 0, s1_id, s2_id),
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

    TEST_CASE("GlobalRoutingTree::optimize_steiner_points - remove redundant steiner") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id
            = tree.insert_steiner_node({20, 0}, s1_id);  // s2 has only one child (none yet)
        std::string t1_id = tree.insert_terminal_node({30, 0}, s2_id);  // s2 now has t1 as child

        CHECK(tree.get_all_steiner_nodes().size() == 2);  // s1, s2
        CHECK(tree.nodes.count(s1_id));
        CHECK(tree.nodes.count(s2_id));

        tree.optimize_steiner_points();

        // s2 should be removed because it has only one child (t1) and a parent (s1)
        CHECK(tree.get_all_steiner_nodes().size() == 1);  // Only s1 remains
        CHECK(tree.nodes.count(s1_id));
        CHECK_FALSE(tree.nodes.count(s2_id));

        // s1 should now directly point to t1
        CHECK(tree.nodes.at(s1_id)->children.size() == 1);
        CHECK(tree.nodes.at(s1_id)->children[0]->id == t1_id);
        CHECK(tree.nodes.at(t1_id)->parent->id == s1_id);
    }

    TEST_CASE("GlobalRoutingTree::optimize_steiner_points - do not remove non-redundant steiner") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string s1_id = tree.insert_steiner_node({10, 0});
        std::string s2_id = tree.insert_steiner_node({20, 0}, s1_id);
        std::string t1_id = tree.insert_terminal_node({30, 0}, s2_id);
        std::string t2_id = tree.insert_terminal_node({20, 10}, s2_id);  // s2 now has two children

        CHECK(tree.get_all_steiner_nodes().size() == 2);  // s1, s2

        tree.optimize_steiner_points();

        // No steiner nodes should be removed
        CHECK(tree.get_all_steiner_nodes().size() == 2);
        CHECK(tree.nodes.count(s1_id));
        CHECK(tree.nodes.count(s2_id));
    }

    TEST_CASE("GlobalRoutingTree::optimize_steiner_points - source node is not removed") {
        GlobalRoutingTree tree(Point{0, 0});
        std::string t1_id = tree.insert_terminal_node({10, 0});  // source has one child (t1)

        CHECK(tree.get_all_steiner_nodes().empty());
        CHECK(tree.get_source()->children.size() == 1);

        tree.optimize_steiner_points();

        CHECK(tree.get_all_steiner_nodes().empty());
        CHECK(tree.get_source()->children.size() == 1);
        CHECK(tree.get_source()->children[0]->id == t1_id);
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
        RoutingNode* source = tree.get_source();
        CHECK(source->children.size() == 1);
        RoutingNode* new_steiner = source->children[0];
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

TEST_CASE("Test route_with_steiner") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(19);
    hgenY.reseed(19);

    std::vector<IntPoint> terminals;
    for (int i = 0; i < 5; ++i) {
        terminals.emplace_back(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop()));
    }
    IntPoint source(static_cast<int>(hgenX.pop()), static_cast<int>(hgenX.pop()));

    GlobalRouter router(source, terminals);
    router.route_with_steiners();

    std::string svg_output = visualize_routing_tree_svg(router.get_tree(), 1000, 1000);
    std::cout << svg_output;

    save_routing_tree_svg(router.get_tree(), "example_route_with_steiner.svg");
}

TEST_CASE("Test route_with_constraints") {
    ildsgen::VdCorput hgenX(3, 7);
    ildsgen::VdCorput hgenY(2, 11);

    hgenX.reseed(19);
    hgenY.reseed(19);

    std::vector<IntPoint> terminals;
    for (int i = 0; i < 5; ++i) {
        terminals.emplace_back(static_cast<int>(hgenX.pop()), static_cast<int>(hgenY.pop()));
    }
    IntPoint source(static_cast<int>(hgenX.pop()), static_cast<int>(hgenX.pop()));

    GlobalRouter router(source, terminals);
    router.route_with_constraints(1.1);

    std::string svg_output = visualize_routing_tree_svg(router.get_tree(), 1000, 1000);
    std::cout << svg_output;

    save_routing_tree_svg(router.get_tree(), "example_route_with_constraint.svg");
}