#include <chrono>
#include <cstdio>
#include <recti/global_router.hpp>
#include <recti/point.hpp>
#include <string>
#include <vector>

using namespace recti;
using IntPoint = Point<int, int>;

int main() {
    for (int n : {128, 512, 1024}) {
        std::vector<IntPoint> terminals;
        terminals.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) {
            terminals.emplace_back((i * 37) % 1000, (i * 91) % 1000);
        }
        IntPoint source(0, 0);

        auto t0 = std::chrono::steady_clock::now();
        GlobalRouter router(source, terminals);
        router.route_with_steiners();
        auto t1 = std::chrono::steady_clock::now();

        const auto& tree = router.get_tree();
        int wirelength = tree.calculate_total_wirelength();

        auto t2 = std::chrono::steady_clock::now();
        std::string structure = tree.get_tree_structure();
        auto t3 = std::chrono::steady_clock::now();

        double route_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        double tree_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
        std::printf("router n=%4d: route %9.2f ms | get_tree_structure %8.2f ms (wl=%d len=%zu)\n",
                    n, route_ms, tree_ms, wirelength, structure.size());
    }
    return 0;
}
