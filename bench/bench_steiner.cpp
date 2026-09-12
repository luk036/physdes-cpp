#define RECTI_STEINER_NO_MAIN
#include <chrono>
#include <cstdio>

#include "../experiments/steiner_forest_grid.cpp"

namespace {
    auto make_pairs(int height, int width) -> std::vector<Pair> {
        std::vector<Pair> pairs;
        pairs.emplace_back(Point(0, 0), Point(height - 1, width - 1));
        pairs.emplace_back(Point(0, width - 1), Point(height - 1, 0));
        pairs.emplace_back(Point(0, width / 2), Point(height - 1, width / 2));
        pairs.emplace_back(Point(height / 2, 0), Point(height / 2, width - 1));
        return pairs;
    }
}  // namespace

int main() {
    for (int n : {8, 16, 32, 48}) {
        auto pairs = make_pairs(n, n);
        SteinerForestGrid solver(n, n, pairs);
        auto t0 = std::chrono::steady_clock::now();
        auto result = solver.compute();
        auto t1 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::printf("steiner %2dx%-2d: %8.2f ms  (edges=%zu cost=%.1f)\n", n, n, ms,
                    result.edges.size(), result.totalCost);
    }
    return 0;
}
