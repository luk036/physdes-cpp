#include <benchmark/benchmark.h>
#include <recti/point.hpp>
#include <recti/dme_algorithm.hpp>
#include <memory>

using namespace recti;
using IntPoint = Point<int, int>;

static void BM_DME_16(benchmark::State& state) {
    std::vector<Sink> sinks;
    for (int i = 0; i < 16; ++i) {
        // Unique positions, spread across 0..500
        int x = i * 33 + 7;
        int y = (i * 17) % 200;
        sinks.emplace_back("s" + std::to_string(i), IntPoint(x, y), 1.0);
    }
    for (auto _ : state) {
        auto calc = std::make_unique<LinearDelayCalculator>(0.5, 0.1);
        auto dme = DMEAlgorithm(sinks, std::move(calc));
        auto root = dme.build_clock_tree();
        benchmark::DoNotOptimize(root);
    }
}
BENCHMARK(BM_DME_16);

static void BM_DME_64(benchmark::State& state) {
    std::vector<Sink> sinks;
    for (int i = 0; i < 64; ++i) {
        // Unique positions, same density range as 16-sink case
        int x = i * 8 + 5;
        int y = (i * 31 + 7) % 500;
        sinks.emplace_back("s" + std::to_string(i), IntPoint(x, y), 1.0);
    }
    for (auto _ : state) {
        auto calc = std::make_unique<LinearDelayCalculator>(0.5, 0.1);
        auto dme = DMEAlgorithm(sinks, std::move(calc));
        auto root = dme.build_clock_tree();
        benchmark::DoNotOptimize(root);
    }
}
BENCHMARK(BM_DME_64);

BENCHMARK_MAIN();
