#pragma once

#include <vector>

#include "dllink.hpp"

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

    constexpr auto operator*() noexcept -> Dllink<size_t>& { return *cur; }

    constexpr auto operator!=(const RDllIterator& other) const noexcept -> bool {
        return cur != other.cur;
    }

    constexpr auto operator==(const RDllIterator& other) const noexcept -> bool {
        return cur == other.cur;
    }

    // For range-based for loop support
    constexpr auto begin() noexcept -> RDllIterator { return RDllIterator{cur->next}; }

    constexpr auto end() noexcept -> RDllIterator { return RDllIterator{stop}; }
};

class RDllist {
  public:
    std::vector<Dllink<size_t>> cycle;

  public:
    explicit RDllist(size_t num_nodes, bool reverse = false) {
        cycle.reserve(10 * num_nodes + 100000);  // must reserve enough space for expansion!
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

    auto from_node(size_t k) -> RDllIterator { return RDllIterator{&cycle.at(k)}; }

    auto begin() -> RDllIterator { return from_node(0); }

    auto end() -> RDllIterator {
        return RDllIterator{nullptr};  // Dummy end iterator
    }
};
