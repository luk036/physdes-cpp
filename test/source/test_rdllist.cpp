#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs

#include <recti/rdllist.hpp>
#include <stdexcept>

TEST_CASE("test_rdllist_init") {
    /*
    Test initialization of RDllist.
    */
    RDllist dll(5);
    for (size_t i = 0; i < 5; ++i) {
        CHECK(dll[i].data == i);
        CHECK(dll[i].next->data == (i + 1) % 5);
        CHECK(dll[i].prev->data == (i - 1 + 5) % 5);
    }
}

TEST_CASE("test_rdllist_init_reverse") {
    /*
    Test initialization of RDllist with reverse=true.
    */
    RDllist dll(5, true);
    for (size_t i = 0; i < 5; ++i) {
        CHECK(dll[i].data == i);
        CHECK(dll[i].next->data == (i - 1 + 5) % 5);
        CHECK(dll[i].prev->data == (i + 1) % 5);
    }
}

TEST_CASE("test_rdllist_getitem") {
    /*
    Test operator[] of RDllist.
    */
    RDllist dll(5);
    CHECK(dll[0].data == 0);
    CHECK(dll[4].data == 4);
    CHECK_THROWS_AS(dll[5], std::out_of_range);
}

TEST_CASE("test_rdllist_iter") {
    /*
    Test iteration of RDllist.
    */
    RDllist dll(5);
    auto it = dll.begin();
    std::vector<size_t> data;
    for (auto& node : it) {
        data.push_back(node.data);
    }
    std::vector<size_t> expected = {1, 2, 3, 4};
    CHECK(data == expected);
}

TEST_CASE("test_rdllist_from_node") {
    /*
    Test from_node of RDllist.
    */
    RDllist dll(5);
    auto it = dll.from_node(2);
    std::vector<size_t> data;
    for (auto& node : it) {
        data.push_back(node.data);
    }
    std::vector<size_t> expected = {3, 4, 0, 1};
    CHECK(data == expected);
}