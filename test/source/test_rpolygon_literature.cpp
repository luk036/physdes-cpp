#include <doctest/doctest.h>

#include <algorithm>
#include <recti/point.hpp>
#include <recti/rpolygon.hpp>
#include <recti/rpolygon_cut.hpp>
#include <vector>

using namespace recti;

static auto even_index(std::vector<Point<int, int>> full) -> std::vector<Point<int, int>> {
    std::vector<Point<int, int>> stored;
    stored.reserve(full.size() / 2);
    for (std::size_t i = 0; i < full.size(); i += 2) {
        stored.push_back(full[i]);
    }
    return stored;
}

TEST_CASE("RPolygon AGPLIB rand-10-1") {
    // AGPLIB/AGPVG, rand-10-1.pol. Couto, de Souza, and de Rezende,
    // "An Exact and Efficient Algorithm for the Orthogonal Art Gallery
    // Problem", SIBGRAPI 2007.
    const auto S = even_index(
        {{4, 1}, {5, 1}, {5, 5}, {3, 5}, {3, 4}, {1, 4}, {1, 2}, {2, 2}, {2, 3}, {4, 3}});
    CHECK(is_canonical<int>(S));

    const auto pieces = rpolygon_cut_convex<int>(S, true);
    REQUIRE_EQ(pieces.size(), 2U);

    std::vector<int> areas;
    int total_area = 0;
    for (const auto& piece : pieces) {
        CHECK(rpolygon_is_convex<int>(piece));
        CHECK(is_canonical<int>(piece));
        const int area = RPolygon<int>(piece).signed_area();
        areas.push_back(area);
        total_area += area;
    }
    std::sort(areas.begin(), areas.end());
    CHECK(areas == std::vector<int>{2, 7});
    CHECK_EQ(total_area, RPolygon<int>(S).signed_area());
}

TEST_CASE("RPolygon AGPLIB rand-12-6") {
    // AGPLIB/AGPVG, rand-12-6.pol. Same source as rand-10-1.
    const auto S = even_index({{4, 1},
                               {6, 1},
                               {6, 6},
                               {5, 6},
                               {5, 4},
                               {2, 4},
                               {2, 5},
                               {1, 5},
                               {1, 2},
                               {3, 2},
                               {3, 3},
                               {4, 3}});
    CHECK(is_canonical<int>(S));

    const auto pieces = rpolygon_cut_convex<int>(S, true);
    REQUIRE_EQ(pieces.size(), 3U);

    std::vector<int> areas;
    int total_area = 0;
    for (const auto& piece : pieces) {
        CHECK(rpolygon_is_convex<int>(piece));
        CHECK(is_canonical<int>(piece));
        const int area = RPolygon<int>(piece).signed_area();
        areas.push_back(area);
        total_area += area;
    }
    std::sort(areas.begin(), areas.end());
    CHECK(areas == std::vector<int>{2, 5, 7});
    CHECK_EQ(total_area, RPolygon<int>(S).signed_area());
}

TEST_CASE("RPolygon bridged-hole fixture") {
    const std::vector<Point<int, int>> S{{4, 1}, {10, 7}, {7, 5}, {9, 2}, {6, 5}, {7, 7}};
    CHECK(is_canonical<int>(S));

    const auto pieces = rpolygon_cut_convex<int>(S, true);
    REQUIRE_EQ(pieces.size(), 3U);

    std::vector<int> areas;
    int total_area = 0;
    for (const auto& piece : pieces) {
        CHECK(rpolygon_is_convex<int>(piece));
        CHECK(is_canonical<int>(piece));
        const int area = RPolygon<int>(piece).signed_area();
        areas.push_back(area);
        total_area += area;
    }
    std::sort(areas.begin(), areas.end());
    CHECK(areas == std::vector<int>{2, 10, 15});
    CHECK_EQ(total_area, RPolygon<int>(S).signed_area());
}
