#include <benchmark/benchmark.h>
#include <recti/polygon.hpp>
#include <recti/rpolygon.hpp>
#include <vector>

using namespace recti;

static void BM_Polygon_Area(benchmark::State& state) {
    auto S = std::vector<Point<int>>{{0, 0}, {100, 0}, {100, 100}, {0, 100}};
    auto poly = Polygon<int>(S);
    for (auto _ : state) {
        benchmark::DoNotOptimize(poly.signed_area_x2());
    }
}
BENCHMARK(BM_Polygon_Area);

static void BM_RPolygon_Area(benchmark::State& state) {
    auto S = std::vector<Point<int>>{{0, 0}, {100, 100}};
    auto rpoly = RPolygon<int>(S);
    for (auto _ : state) {
        benchmark::DoNotOptimize(rpoly.signed_area());
    }
}
BENCHMARK(BM_RPolygon_Area);

BENCHMARK_MAIN();
