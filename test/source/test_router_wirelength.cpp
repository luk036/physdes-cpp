#include <doctest/doctest.h>

#include <array>
#include <lds/ilds.hpp>
#include <recti/global_router.hpp>
#include <recti/interval.hpp>
#include <recti/point.hpp>
#include <vector>

using namespace recti;

namespace {
    using IntPoint = Point<int, int>;

    auto gen(size_t n, unsigned seed) -> std::pair<IntPoint, std::vector<IntPoint>> {
        ilds::VdCorput<3> hx(7);
        ilds::VdCorput<2> hy(11);
        hx.reseed(seed);
        hy.reseed(seed);
        std::vector<IntPoint> t;
        t.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            t.emplace_back(static_cast<int>(hx.pop()), static_cast<int>(hy.pop()));
        }
        // Pop sequentially: argument evaluation order is unspecified and differs
        // between compilers (MSVC right-to-left, AppleClang left-to-right).
        auto source_x = static_cast<int>(hx.pop());
        auto source_y = static_cast<int>(hx.pop());
        IntPoint s(source_x, source_y);
        return {s, t};
    }

    struct Case {
        unsigned seed;
        int n;
        int steiner;
        int constraint;
        int keepout;
    };

    // Pinned totals captured from the reference implementation; guards the
    // subtree-bounding-box pruning against any change in the chosen tree.
    const std::array<Case, 12> CASES = {{
        {19, 7, 5329, 6679, 5329},
        {19, 20, 9123, 25060, 9123},
        {19, 50, 14688, 60829, 15062},
        {23, 7, 5437, 7269, 5437},
        {23, 20, 9457, 20997, 9459},
        {23, 50, 15007, 47337, 16248},
        {42, 7, 5608, 9257, 5608},
        {42, 20, 8476, 17444, 8995},
        {42, 50, 14776, 29616, 15138},
        {101, 7, 5342, 5887, 5342},
        {101, 20, 8877, 21832, 8877},
        {101, 50, 15015, 41603, 14823},
    }};
}  // namespace

TEST_CASE("Global router deterministic wirelength") {
    for (const auto& c : CASES) {
        auto [src, terms] = gen(static_cast<size_t>(c.n), c.seed);

        GlobalRouter<IntPoint> r1(src, terms);
        r1.route_with_steiners();
        CHECK_EQ(r1.get_tree().calculate_total_wirelength(), c.steiner);

        GlobalRouter<IntPoint> r2(src, terms);
        r2.route_with_constraints(1.0);
        CHECK_EQ(r2.get_tree().calculate_total_wirelength(), c.constraint);

        std::vector<GlobalRouter<IntPoint>::Keepout> keepouts;
        keepouts.push_back(
            GlobalRouter<IntPoint>::Keepout{Interval<int>(400, 600), Interval<int>(400, 600)});
        GlobalRouter<IntPoint> r3(src, terms, keepouts);
        r3.route_with_steiners();
        CHECK_EQ(r3.get_tree().calculate_total_wirelength(), c.keepout);
    }
}
