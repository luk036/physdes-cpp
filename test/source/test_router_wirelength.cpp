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
        IntPoint s(static_cast<int>(hx.pop()), static_cast<int>(hx.pop()));
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
        {19, 7, 5635, 6683, 5635},
        {19, 20, 9101, 11792, 9101},
        {19, 50, 14666, 59789, 15040},
        {23, 7, 4638, 7108, 4638},
        {23, 20, 9640, 31733, 9640},
        {23, 50, 15108, 63443, 16671},
        {42, 7, 5951, 6845, 5951},
        {42, 20, 9166, 15404, 9166},
        {42, 50, 14536, 61326, 14518},
        {101, 7, 5317, 6213, 5317},
        {101, 20, 9159, 24152, 9159},
        {101, 50, 15065, 84604, 14977},
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
