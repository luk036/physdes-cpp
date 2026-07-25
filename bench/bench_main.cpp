#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <recti/polygon.hpp>
#include <recti/rpolygon.hpp>
#include <vector>

using namespace recti;

int main() {
    ankerl::nanobench::Bench bench;
    bench.title("Polygon Area Benchmarks").unit("op").warmup(100).epochs(50);

    bench.run("Polygon signed_area_x2", [&] {
        auto S = std::vector<Point<int>>{{0, 0}, {100, 0}, {100, 100}, {0, 100}};
        auto poly = Polygon<int>(S);
        ankerl::nanobench::doNotOptimizeAway(poly.signed_area_x2());
    });

    bench.run("RPolygon signed_area", [&] {
        auto S = std::vector<Point<int>>{{0, 0}, {100, 100}};
        auto rpoly = RPolygon<int>(S);
        ankerl::nanobench::doNotOptimizeAway(rpoly.signed_area());
    });
}
