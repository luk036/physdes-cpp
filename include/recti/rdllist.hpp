#pragma once

#include <cstddef>  // for size_t
#include <vector>

#include "dllink.hpp"

/**
 * @brief Iterator for RDllist circular doubly-linked list
 *
 * This class provides iterator functionality for traversing the nodes
 * in an RDllist. It supports forward iteration and can be used with
 * range-based for loops.
 */
class RDllIterator {
  private:
    Dllink<size_t>* cur;
    Dllink<size_t>* stop;

  public:
    constexpr explicit RDllIterator(Dllink<size_t>* node) noexcept : cur{node}, stop{node} {}

    constexpr auto operator++() noexcept -> RDllIterator& {
        cur = cur->next;
        return *this;
    }

    constexpr auto operator*() const noexcept -> const Dllink<size_t>& { return *cur; }

    constexpr auto operator!=(const RDllIterator& other) const noexcept -> bool {
        return cur != other.cur;
    }

    constexpr auto operator==(const RDllIterator& other) const noexcept -> bool {
        return cur == other.cur;
    }

    // For range-based for loop support
    constexpr auto begin() const noexcept -> RDllIterator { return RDllIterator{cur->next}; }

    constexpr auto end() const noexcept -> RDllIterator { return RDllIterator{stop}; }
};

/**
 * @brief Circular doubly-linked list implementation
 *
 * This class implements a circular doubly-linked list using the Dllink
 * template with size_t as the data type. It provides efficient node
 * management and supports both forward and reverse ordering.
 */
class RDllist {
  public:
    std::vector<Dllink<size_t>> cycle;  ///< Storage for all list nodes

  public:
    /**
     * @brief Construct a new RDllist object
     *
     * Creates a circular doubly-linked list with the specified number of nodes.
     * The nodes are linked together in a circular fashion, with the last node
     * pointing back to the first.
     *
     * @param[in] num_nodes The number of nodes to create in the list
     * @param[in] reverse If true, creates the list in reverse order
     */
    explicit RDllist(size_t num_nodes, bool reverse = false) {
        cycle.reserve(3 * num_nodes);  // must reserve enough space for expansion!
        for (size_t k = 0; k < num_nodes; ++k) {
            cycle.emplace_back(Dllink<size_t>(k));
        }

        Dllink<size_t>* dl2 = &cycle.back();

        if (!reverse) {
            for (auto& dl1 : cycle) {
                dl2->next = &dl1;
                dl1.prev = dl2;
                dl2 = &dl1;
            }
        } else {
            for (auto& dl1 : cycle) {
                dl2->prev = &dl1;
                dl1.next = dl2;
                dl2 = &dl1;
            }
        }
    }

    auto operator[](size_t k) -> Dllink<size_t>& { return cycle.at(k); }

    auto operator[](size_t k) const -> const Dllink<size_t>& { return cycle.at(k); }

    auto from_node(size_t k) -> RDllIterator { return RDllIterator{&cycle.at(k)}; }

    auto from_node(size_t k) const -> RDllIterator {
        return RDllIterator{const_cast<Dllink<size_t>*>(&cycle.at(k))};
    }

    auto begin() -> RDllIterator { return from_node(0); }

    auto begin() const -> RDllIterator { return from_node(0); }

    auto end() -> RDllIterator {
        return RDllIterator{nullptr};  // Dummy end iterator
    }

    auto end() const -> RDllIterator { return RDllIterator{nullptr}; }
};
