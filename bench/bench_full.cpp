#include <chrono>
#include <cstdio>
#include <vector>
#include <memory>
#include <recti/point.hpp>
#include <recti/dme_algorithm.hpp>

using namespace recti;
using IntPoint = Point<int, int>;

static void bench_dme(const std::vector<Sink>& sinks, const char* label) {
    int n = 1000;
    // warm-up
    for (int i = 0; i < 100; ++i) {
        auto calc = std::make_unique<LinearDelayCalculator>(0.5, 0.1);
        auto dme = DMEAlgorithm(sinks, std::move(calc));
        volatile auto r = dme.build_clock_tree();
        (void)r;
    }
    size_t accum = 0;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < n; ++i) {
        auto calc = std::make_unique<LinearDelayCalculator>(0.5, 0.1);
        auto dme = DMEAlgorithm(sinks, std::move(calc));
        accum += dme.build_clock_tree();
    }
    volatile size_t dummy = accum;
    (void)dummy;
    auto ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start)
                  .count()
              / n;
    std::printf("  %-40s %8.1f ns/op  (%zu sinks)\n", label, ns, sinks.size());
}

int main() {
    std::printf("=== C++ (physdes-cpp) — Full Algorithm Benchmarks ===\n\n");

    // DME Algorithm
    {
        std::vector<Sink> sinks;
        for (int i = 0; i < 16; ++i) {
            int x = i * 33 + 7;
            int y = (i * 17) % 200;
            sinks.emplace_back("s" + std::to_string(i), IntPoint(x, y), 1.0);
        }
        bench_dme(sinks, "DME build_clock_tree (16 sinks)");
    }
    {
        std::vector<Sink> sinks;
        for (int i = 0; i < 64; ++i) {
            int x = i * 8 + 5;
            int y = (i * 31 + 7) % 500;
            sinks.emplace_back("s" + std::to_string(i), IntPoint(x, y), 1.0);
        }
        bench_dme(sinks, "DME build_clock_tree (64 sinks)");
    }

    std::printf("\n  Note: GlobalRouter has template compat issue with Point<int,int>,\n");
    std::printf("  C++ Steiner Forest is in experiments/ (not library), skipped.\n");
}
