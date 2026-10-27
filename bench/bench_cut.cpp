#include <algorithm>
#include <chrono>
#include <cstdio>
#include <numeric>
#include <span>
#include <vector>

#include <recti/polygon.hpp>
#include <recti/rpolygon.hpp>
#include <recti/rpolygon_cut.hpp>

using namespace recti;
using IntPoint = Point<int>;

namespace {

std::vector<IntPoint> twelve_point_stored() {
    return {{-1, 1}, {0, 0}, {1, -1}, {3, 0}, {4, 1}, {6, 2},
            {5, 5},  {4, 4}, {3, 2},  {2, 3}, {1, 2}, {0, 3}};
}

std::vector<IntPoint> comb(int k) {
    const int w = 2 * k + 1;
    const int h = 2;
    std::vector<IntPoint> full;
    full.reserve(4 * k + 5);
    full.emplace_back(0, 0);
    full.emplace_back(w, 0);
    full.emplace_back(w, h);
    int x = w;
    for (int i = 0; i < k; ++i) {
        --x;
        full.emplace_back(x, h);
        full.emplace_back(x, h - 1);
        --x;
        full.emplace_back(x, h - 1);
        full.emplace_back(x, h);
    }
    full.emplace_back(0, h);
    return full;
}

long long area_x2(std::span<const IntPoint> full) {
    Polygon<int> polygon{full};
    return polygon.signed_area_x2();
}

bool all_convex(const std::vector<std::vector<IntPoint>>& pieces, bool pieces_are_even_index) {
    for (const auto& piece : pieces) {
        if (pieces_are_even_index) {
            const auto full = rpolygon_expand<int>(piece);
            if (!rpolygon_is_convex<int>(full)) return false;
        } else if (!rpolygon_is_convex<int>(piece)) {
            return false;
        }
    }
    return true;
}

long long total_area_x2(const std::vector<std::vector<IntPoint>>& pieces, bool even_index) {
    long long total = 0;
    for (const auto& piece : pieces) {
        if (even_index) {
            const auto full = rpolygon_expand<int>(piece);
            total += area_x2(full);
        } else {
            total += area_x2(piece);
        }
    }
    return total;
}

template <typename F> double median_us(F&& f, int reps) {
    std::vector<double> samples;
    samples.reserve(static_cast<std::size_t>(reps));
    for (int i = 0; i < reps; ++i) {
        const auto start = std::chrono::steady_clock::now();
        volatile long long sink = static_cast<long long>(f());
        (void)sink;
        const auto stop = std::chrono::steady_clock::now();
        samples.push_back(std::chrono::duration<double, std::micro>(stop - start).count());
    }
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

void run_case(const char* name, const std::vector<IntPoint>& stored, bool is_ccw, int reps) {
    const auto full = rpolygon_expand<int>(stored);
    const long long expected = area_x2(full);
    const auto even_pieces = rpolygon_cut_convex<int>(stored, is_ccw);
    const auto full_pieces = rpolygon_cut_convex_full<int>(full, is_ccw);

    const long long even_area = total_area_x2(even_pieces, true);
    const long long full_area = total_area_x2(full_pieces, false);
    const bool even_ok = even_area == expected && all_convex(even_pieces, true);
    const bool full_ok = full_area == expected && all_convex(full_pieces, false);

    if (!full_ok) {
        std::printf("DIAG %s: expected=%lld full_area=%lld\n", name, expected, full_area);
        for (std::size_t i = 0; i < full_pieces.size(); ++i) {
            std::printf("  piece %zu: k=%zu area=%lld convex=%d\n", i, full_pieces[i].size(),
                        area_x2(full_pieces[i]), rpolygon_is_convex<int>(full_pieces[i]) ? 1 : 0);
        }
    }

    const double even_us = median_us(
        [&] { return static_cast<long long>(rpolygon_cut_convex<int>(stored, is_ccw).size()); },
        reps);
    const double full_us = median_us(
        [&] {
            return static_cast<long long>(rpolygon_cut_convex_full<int>(full, is_ccw).size());
        },
        reps);

    std::printf("| %s | %zu | %zu | %zu | %zu | %lld | %s | %s | %.1f | %.1f | %.2fx |\n", name,
                stored.size(), full.size(), even_pieces.size(), full_pieces.size(), expected,
                even_ok ? "ok" : "FAIL", full_ok ? "ok" : "FAIL", even_us, full_us,
                full_us / even_us);
}

}  // namespace

int main() {
    std::printf("| fixture | m | n | even pieces | full pieces | area x2 | even check | full "
                "check | even cut (us) | full cut (us) | ratio |\n");
    std::printf("|---|---:|---:|---:|---:|---:|---|---|---:|---:|---:|\n");

    run_case("twelve-point", twelve_point_stored(), true, 200);
    for (int k : {2, 4, 8, 16, 32, 64}) {
        const auto full = comb(k);
        std::vector<IntPoint> stored;
        stored.reserve(full.size() / 2);
        for (std::size_t i = 0; i < full.size(); i += 2) stored.push_back(full[i]);
        run_case("comb", stored, true, 20);
    }
    return 0;
}
