#include <chrono>
#include <cstdio>
#include <vector>
#include <recti/polygon.hpp>
#include <recti/rpolygon.hpp>

using namespace recti;

int main() {
    std::printf("=== C++ (physdes-cpp) — Polygon Area Benchmarks ===\n\n");

    // Polygon signed_area_x2
    {
        std::vector<Polygon<int>> polys;
        for (int i = 0; i < 1000; ++i) {
            polys.emplace_back(std::vector<Point<int>>{
                {i * 10, i * 10},
                {i * 10 + 100, i * 10},
                {i * 10 + 100, i * 10 + 100},
                {i * 10, i * 10 + 100},
            });
        }
        long long accum = 0;
        auto start = std::chrono::steady_clock::now();
        for (int j = 0; j < 100000; ++j) {
            for (const auto& p : polys) {
                accum += p.signed_area_x2();
            }
        }
        auto ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start)
                      .count()
                  / (100000.0 * polys.size());
        std::printf("  %-35s %8.2f ns/op  (accum=%lld)\n", "Polygon signed_area_x2", ns, accum);
    }

    // RPolygon signed_area
    {
        std::vector<RPolygon<int>> polys;
        for (int i = 0; i < 1000; ++i) {
            polys.emplace_back(std::vector<Point<int>>{
                {i * 10, i * 10},
                {i * 10 + 100, i * 10 + 100},
            });
        }
        long long accum = 0;
        auto start = std::chrono::steady_clock::now();
        for (int j = 0; j < 100000; ++j) {
            for (const auto& p : polys) {
                accum += p.signed_area();
            }
        }
        auto ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start)
                      .count()
                  / (100000.0 * polys.size());
        std::printf("  %-35s %8.2f ns/op  (accum=%lld)\n", "RPolygon signed_area", ns, accum);
    }
}
