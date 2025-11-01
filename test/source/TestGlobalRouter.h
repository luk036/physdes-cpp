#pragma once

#include <doctest/doctest.h>
#include <recti/global_router.hpp>

TEST_SUITE("RoutingNode") {
    TEST_CASE_TEMPLATE("RoutingNode::remove_child - existing child", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        RoutingNode parent("parent", NodeType::SOURCE, Point{});
        RoutingNode child1("child1", NodeType::TERMINAL, Point{});
        RoutingNode child2("child2", NodeType::TERMINAL, Point{});

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

    TEST_CASE_TEMPLATE("RoutingNode::remove_child - non-existent child", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        RoutingNode parent("parent", NodeType::SOURCE, Point{});
        RoutingNode child1("child1", NodeType::TERMINAL, Point{});
        RoutingNode child2("child2", NodeType::TERMINAL, Point{});

        parent.add_child(&child1);

        CHECK(parent.children.size() == 1);
        CHECK(child1.parent == &parent);

        parent.remove_child(&child2);  // Try to remove a child that was never added

        CHECK(parent.children.size() == 1);  // Size should remain 1
        CHECK(parent.children[0] == &child1);
        CHECK(child1.parent == &parent);
        CHECK(child2.parent == nullptr);  // Should still be nullptr
    }

    TEST_CASE_TEMPLATE("RoutingNode::remove_child - from empty children list", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        RoutingNode parent("parent", NodeType::SOURCE, Point{});
        RoutingNode child("child", NodeType::TERMINAL, Point{});

        CHECK(parent.children.empty());

        parent.remove_child(&child);  // Remove from empty list

        CHECK(parent.children.empty());
        CHECK(child.parent == nullptr);
    }
}

TEST_SUITE("GTree") {
    TEST_CASE_TEMPLATE("GTree::insert_steiner_node - invalid parent_id", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        CHECK_THROWS_AS(tree.insert_steiner_node(Point{}, "non_existent_parent"),
                        std::runtime_error);
    }

    TEST_CASE_TEMPLATE("GTree::insert_terminal_node - invalid parent_id", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        CHECK_THROWS_AS(tree.insert_terminal_node(Point{}, "non_existent_parent"),
                        std::runtime_error);
    }

    TEST_CASE_TEMPLATE("GTree::insert_node_on_branch - basic insertion", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        std::string s1_id = tree.insert_steiner_node(Point{});
        std::string s2_id = tree.insert_steiner_node(Point{}, s1_id);

        CHECK(tree.nodes.at(s1_id)->children.size() == 1);
        CHECK(tree.nodes.at(s1_id)->children[0]->id == s2_id);

        std::string new_s_id
            = tree.insert_node_on_branch(NodeType::STEINER, Point{}, s1_id, s2_id);

        CHECK(tree.nodes.at(s1_id)->children.size() == 1);
        CHECK(tree.nodes.at(s1_id)->children[0]->id == new_s_id);
        CHECK(tree.nodes.at(new_s_id)->children.size() == 1);
        CHECK(tree.nodes.at(new_s_id)->children[0]->id == s2_id);
        CHECK(tree.nodes.at(s2_id)->parent->id == new_s_id);
    }

    TEST_CASE_TEMPLATE("GTree::insert_node_on_branch - invalid branch_start_id", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        std::string s1_id = tree.insert_steiner_node(Point{});
        std::string s2_id = tree.insert_steiner_node(Point{}, s1_id);
        CHECK_THROWS_AS(
            tree.insert_node_on_branch(NodeType::STEINER, Point{}, "non_existent", s2_id),
            std::runtime_error);
    }

    TEST_CASE_TEMPLATE("GTree::insert_node_on_branch - invalid branch_end_id", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        std::string s1_id = tree.insert_steiner_node(Point{});
        std::string s2_id = tree.insert_steiner_node(Point{}, s1_id);
        CHECK_THROWS_AS(
            tree.insert_node_on_branch(NodeType::STEINER, Point{}, s1_id, "non_existent"),
            std::runtime_error);
    }

    TEST_CASE_TEMPLATE("GTree::insert_node_on_branch - end_node not child of start_node", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        std::string s1_id = tree.insert_steiner_node(Point{});
        std::string s2_id = tree.insert_steiner_node(Point{});  // s2 is not child of s1

        CHECK_THROWS_AS(tree.insert_node_on_branch(NodeType::STEINER, Point{}, s1_id, s2_id),
                        std::runtime_error);
    }

    TEST_CASE_TEMPLATE("GTree::find_path_to_source - basic path", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        std::string s1_id = tree.insert_steiner_node(Point{});
        std::string s2_id = tree.insert_steiner_node(Point{}, s1_id);
        std::string t1_id = tree.insert_terminal_node(Point{}, s2_id);

        auto path = tree.find_path_to_source(t1_id);
        CHECK(path.size() == 4);  // source -> s1 -> s2 -> t1
        CHECK(path[0]->id == "source");
        CHECK(path[1]->id == s1_id);
        CHECK(path[2]->id == s2_id);
        CHECK(path[3]->id == t1_id);
    }

    TEST_CASE_TEMPLATE("GTree::find_path_to_source - source node", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        auto path = tree.find_path_to_source("source");
        CHECK(path.size() == 1);
        CHECK(path[0]->id == "source");
    }

    TEST_CASE_TEMPLATE("GTree::find_path_to_source - non-existent node", Point, recti::Point<int, int>, recti::Point<recti::Point<int, int>, int>) {
        using namespace recti;
        GlobalRoutingTree tree(Point{});
        CHECK_THROWS_AS(tree.find_path_to_source("non_existent"), std::runtime_error);
    }
}

