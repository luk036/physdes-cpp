#include <doctest/doctest.h>

#include <cstddef>
#include <recti/point.hpp>
#include <recti/polygon.hpp>
#include <recti/rpolygon.hpp>
#include <recti/rpolygon_cut.hpp>
#include <vector>

using namespace recti;

namespace {

auto twelve_point_full() -> std::vector<Point<int>> {
    return {{-1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, -1}, {3, -1}, {3, 0}, {4, 0},
            {4, 1},  {6, 1}, {6, 2}, {5, 2}, {5, 5}, {4, 5},  {4, 4}, {3, 4},
            {3, 2},  {2, 2}, {2, 3}, {1, 3}, {1, 2}, {0, 2},  {0, 3}, {-1, 3}};
}

auto comb_full(int k) -> std::vector<Point<int>> {
    const int w = 2 * k + 1;
    const int h = 2;
    std::vector<Point<int>> full;
    full.emplace_back(0, 0);
    full.emplace_back(w, 0);
    full.emplace_back(w, h);
    int x = w;
    for (int i = 0; i < k; ++i) {
        --x;
        full.emplace_back(x, h);
        full.emplace_back(x, h - 1);
        --x;
        full.emplace_back(x, h - 1);
        full.emplace_back(x, h);
    }
    full.emplace_back(0, h);
    return full;
}

auto even_index(std::span<const Point<int>> full) -> std::vector<Point<int>> {
    std::vector<Point<int>> stored;
    stored.reserve(full.size() / 2);
    for (std::size_t i = 0; i < full.size(); i += 2) {
        stored.push_back(full[i]);
    }
    return stored;
}

auto area_x2(std::span<const Point<int>> points) -> int {
    return Polygon<int>{points}.signed_area_x2();
}

void check_partition(std::span<const Point<int>> full) {
    const bool is_ccw = area_x2(full) > 0;
    const auto pieces = rpolygon_cut_convex_full<int>(full, is_ccw);
    CHECK(!pieces.empty());

    int total = 0;
    for (const auto& piece : pieces) {
        CHECK(piece.size() >= 4);
        CHECK(rpolygon_is_convex<int>(piece));
        for (std::size_t i = 0; i < piece.size(); ++i) {
            const auto& a = piece[i];
            const auto& b = piece[(i + 1) % piece.size()];
            const bool duplicate = a.xcoord() == b.xcoord() && a.ycoord() == b.ycoord();
            CHECK_FALSE(duplicate);
        }
        total += area_x2(piece);
    }
    CHECK_EQ(total, area_x2(full));
}

}  // namespace

TEST_CASE("rpolygon_expand restores the full boundary") {
    const auto full = twelve_point_full();
    const auto stored = even_index(full);
    const auto expanded = rpolygon_expand<int>(stored);
    CHECK_EQ(expanded.size(), full.size());
    for (std::size_t i = 0; i < full.size(); ++i) {
        CHECK_EQ(expanded[i].xcoord(), full[i].xcoord());
        CHECK_EQ(expanded[i].ycoord(), full[i].ycoord());
    }
}

TEST_CASE("full-vertex convex decomposition preserves area and convexity") {
    check_partition(twelve_point_full());
    for (int k : {1, 2, 4, 8, 16}) {
        check_partition(comb_full(k));
    }
}

TEST_CASE("full-vertex and even-index convex cuts have the same area") {
    const auto full = twelve_point_full();
    const auto stored = even_index(full);
    const bool is_ccw = area_x2(full) > 0;

    const auto even_pieces = rpolygon_cut_convex<int>(stored, is_ccw);
    const auto full_pieces = rpolygon_cut_convex_full<int>(full, is_ccw);

    int even_total = 0;
    for (const auto& piece : even_pieces) {
        even_total += area_x2(rpolygon_expand<int>(piece));
    }
    int full_total = 0;
    for (const auto& piece : full_pieces) {
        full_total += area_x2(piece);
    }
    CHECK_EQ(even_total, full_total);
    CHECK_EQ(even_total, area_x2(full));
}
