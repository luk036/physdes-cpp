#include <doctest/doctest.h>
#include <fmt/core.h>

#include <lds/ilds.hpp>
#include <random>
#include <recti/point.hpp>
#include <recti/polygon.hpp>
#include <recti/rpolygon.hpp>
#include <recti/rpolygon_cut.hpp>
#include <recti/rpolygon_hull.hpp>
#include <vector>

using namespace recti;

namespace {
    auto generate_random_rpolygon(size_t num_points, int max_coord) -> std::vector<Point<int>> {
        std::vector<Point<int>> S;
        std::mt19937 gen(12345);
        std::uniform_int_distribution<> distrib_coord(0, max_coord);

        S.reserve(num_points);
        for (size_t i = 0; i < num_points; ++i) {
            S.emplace_back(distrib_coord(gen), distrib_coord(gen));
        }
        return create_test_rpolygon(S.begin(), S.end());
    }
}  // namespace

TEST_SUITE("RPolygon Stress Tests") {
    TEST_CASE("Large polygon hull and cut") {
        const size_t num_points = 1000;
        const int max_coord = 10000;

        auto P = generate_random_rpolygon(num_points, max_coord);
        bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);

        SUBCASE("Convex hull") {
            auto hull = rpolygon_make_convex_hull<int>(P, is_anticlockwise);
            CHECK(rpolygon_is_convex<int>(hull));
            CHECK(is_canonical<int>(hull));
        }

        SUBCASE("X-monotone hull") {
            auto hull = rpolygon_make_xmonotone_hull<int>(P, is_anticlockwise);
            CHECK(rpolygon_is_xmonotone<int>(hull));
            CHECK(is_canonical<int>(hull));
        }

        SUBCASE("Y-monotone hull") {
            auto hull = rpolygon_make_ymonotone_hull<int>(P, is_anticlockwise);
            CHECK(rpolygon_is_ymonotone<int>(hull));
            CHECK(is_canonical<int>(hull));
        }
    }

    TEST_CASE("Degenerate polygons") {
        SUBCASE("Collinear points") {
            std::vector<Point<int>> S;
            for (int i = 0; i < 100; ++i) {
                S.emplace_back(i, i);
                S.emplace_back(i + 1, i);
            }
            auto P = create_test_rpolygon(S.begin(), S.end());
            bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
            auto hull = rpolygon_make_convex_hull<int>(P, is_anticlockwise);
            CHECK(rpolygon_is_convex<int>(hull));
            CHECK(is_canonical<int>(hull));
        }

        // SUBCASE("Duplicate points") {
        //     std::vector<Point<int>> S;
        //     for (int i = 0; i < 100; ++i) {
        //         S.emplace_back(i, i);
        //         S.emplace_back(i, i);
        //     }
        //     auto P = create_test_rpolygon(S.begin(), S.end());
        //     bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);
        //     auto hull = rpolygon_make_convex_hull<int>(P, is_anticlockwise);
        //     CHECK(rpolygon_is_convex<int>(hull));
        // }
    }
}

TEST_SUITE("RPolygon Stress Tests") {
    TEST_CASE("Large polygon hull and cut") {
        const size_t num_points = 100;
        const int max_coord = 10000;

        auto P = generate_random_rpolygon(num_points, max_coord);
        bool is_anticlockwise = rpolygon_is_anticlockwise<int>(P);

        SUBCASE("Convex cut") {
            auto L = rpolygon_cut_convex<int>(P, is_anticlockwise);
            for (const auto& C : L) {
                CHECK(rpolygon_is_convex<int>(C));
                CHECK(is_canonical<int>(C));
            }
        }
    }
}

TEST_CASE("Full-vertex convex cut on a random polygon") {
    const auto stored = generate_random_rpolygon(200, 2000);
    const auto polygon = rpolygon_expand<int>(stored);
    const auto expected = Polygon<int>{polygon}.signed_area_x2();
    const bool is_anticlockwise = expected > 0;

    const auto pieces = rpolygon_cut_convex_full<int>(polygon, is_anticlockwise);
    CHECK(!pieces.empty());
    int total = 0;
    for (const auto& piece : pieces) {
        CHECK(rpolygon_is_convex<int>(piece));
        total += Polygon<int>{piece}.signed_area_x2();
    }
    CHECK_EQ(total, expected);
}
