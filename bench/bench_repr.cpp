#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <span>
#include <vector>

#include <recti/polygon.hpp>
#include <recti/rpolygon.hpp>

using namespace recti;
using IntPoint = Point<int>;

namespace {

std::vector<IntPoint> staircase(int k) {
    std::vector<IntPoint> full;
    full.reserve(2 * k + 2);
    full.emplace_back(0, 0);
    full.emplace_back(k, 0);
    int y = 0;
    for (int x = k; x > 0; --x) {
        ++y;
        full.emplace_back(x, y);
        full.emplace_back(x - 1, y);
    }
    return full;
}

long long bbox_span(std::span<const IntPoint> pts) {
    int xmin = pts.front().xcoord();
    int xmax = xmin;
    int ymin = pts.front().ycoord();
    int ymax = ymin;
    for (const auto& p : pts) {
        xmin = std::min(xmin, p.xcoord());
        xmax = std::max(xmax, p.xcoord());
        ymin = std::min(ymin, p.ycoord());
        ymax = std::max(ymax, p.ycoord());
    }
    return static_cast<long long>(xmax - xmin) + (ymax - ymin);
}

long long canonical_full(std::span<const IntPoint> pts) {
    const std::size_t n = pts.size();
    long long bad = 0;
    for (std::size_t i = 0; i < n; ++i) {
        const auto& a = pts[(i + n - 1) % n];
        const auto& b = pts[i];
        const auto& c = pts[(i + 1) % n];
        bad += (a.xcoord() == b.xcoord() && b.xcoord() == c.xcoord())
               || (a.ycoord() == b.ycoord() && b.ycoord() == c.ycoord());
    }
    return bad;
}

template <typename F> double ns_per_op(F&& f) {
    constexpr long long target_ns = 20'000'000;
    std::vector<double> samples;
    samples.reserve(7);
    long long reps = 1;
    for (int trial = 0; trial < 7; ++trial) {
        const auto start = std::chrono::steady_clock::now();
        volatile long long sink = 0;
        for (long long i = 0; i < reps; ++i) sink += static_cast<long long>(f());
        const auto stop = std::chrono::steady_clock::now();
        (void)sink;
        const auto elapsed
            = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
        samples.push_back(static_cast<double>(elapsed) / static_cast<double>(reps));
        if (elapsed < target_ns) reps *= 2;
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

}  // namespace

int main() {
    std::printf("| m | n | area stored (ns) | area full (ns) | ratio | bbox stored | bbox full "
                "| canon stored | canon full | expand (ns) | bytes stored | bytes full | mem ratio |\n");
    std::printf("|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|\n");

    for (int m : {8, 32, 128, 512, 2048, 8192}) {
        const int k = m - 1;
        auto full = staircase(k);
        std::vector<IntPoint> stored;
        stored.reserve(m);
        for (std::size_t i = 0; i < full.size(); i += 2) stored.push_back(full[i]);

        RPolygon<int> rp{std::span<const IntPoint>{stored}};
        Polygon<int> pg{std::span<const IntPoint>{full}};
        if (rp.signed_area() * 2 != pg.signed_area_x2()) {
            std::printf("area mismatch at m=%d: %d vs %d\n", m, rp.signed_area(),
                        pg.signed_area_x2());
            return 1;
        }

        const double area_s = ns_per_op([&] { return rp.signed_area(); });
        const double area_f = ns_per_op([&] { return pg.signed_area_x2(); });
        const double bbox_s = ns_per_op([&] { return bbox_span(stored); });
        const double bbox_f = ns_per_op([&] { return bbox_span(full); });
        const double can_s = ns_per_op([&] { return rp.is_canonical() ? 1 : 0; });
        const double can_f = ns_per_op([&] { return canonical_full(full); });
        const double expand = ns_per_op([&] {
            auto v = rpolygon_expand<int>(stored);
            return static_cast<long long>(v.size());
        });

        const long long bytes_s = static_cast<long long>(
            sizeof(RPolygon<int>) + (stored.size() - 1) * sizeof(Vector2<int>));
        const long long bytes_f = static_cast<long long>(
            sizeof(Polygon<int>) + (full.size() - 1) * sizeof(Vector2<int>));

        std::printf("| %d | %d | %.2f | %.2f | %.2fx | %.2f | %.2f | %.2f | %.2f "
                    "| %.2f | %lld | %lld | %.2fx |\n",
                    m, 2 * m, area_s, area_f, area_f / area_s, bbox_s, bbox_f, can_s, can_f,
                    expand, bytes_s, bytes_f, static_cast<double>(bytes_f) / bytes_s);
    }

    std::printf("\nper-point: sizeof(Point<int>)=%zu B, sizeof(Vector2<int>)=%zu B, "
                "sizeof(RPolygon<int>)=%zu B, sizeof(Polygon<int>)=%zu B\n",
                sizeof(IntPoint), sizeof(Vector2<int>), sizeof(RPolygon<int>),
                sizeof(Polygon<int>));
    return 0;
}
