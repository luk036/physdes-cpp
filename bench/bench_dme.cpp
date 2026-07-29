#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <memory>
#include <recti/dme_algorithm.hpp>
#include <recti/point.hpp>

using namespace recti;
using IntPoint = Point<int, int>;

int main() {
    ankerl::nanobench::Bench bench;
    bench.title("DME Algorithm Benchmarks").unit("op").warmup(100).epochs(50);

    bench.run("DME_16", [&] {
        std::vector<Sink> sinks;
        for (int i = 0; i < 16; ++i) {
            int x = i * 33 + 7;
            int y = (i * 17) % 200;
            sinks.emplace_back("s" + std::to_string(i), IntPoint(x, y), 1.0);
        }
        auto calc = std::make_unique<LinearDelayCalculator>(0.5, 0.1);
        auto dme = DMEAlgorithm(sinks, std::move(calc));
        auto root = dme.build_clock_tree();
        ankerl::nanobench::doNotOptimizeAway(root);
    });

    bench.run("DME_64", [&] {
        std::vector<Sink> sinks;
        for (int i = 0; i < 64; ++i) {
            int x = i * 8 + 5;
            int y = (i * 31 + 7) % 500;
            sinks.emplace_back("s" + std::to_string(i), IntPoint(x, y), 1.0);
        }
        auto calc = std::make_unique<LinearDelayCalculator>(0.5, 0.1);
        auto dme = DMEAlgorithm(sinks, std::move(calc));
        auto root = dme.build_clock_tree();
        ankerl::nanobench::doNotOptimizeAway(root);
    });
}
