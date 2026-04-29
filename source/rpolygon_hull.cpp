#include <algorithm>
#include <functional>
#include <recti/rdllist.hpp>
#include <recti/rpolygon.hpp>
#include <recti/rpolygon_hull.hpp>

namespace recti {
    template <typename T>
    auto rpolygon_make_monotone_hull(std::span<const Point<T>> pointset, bool is_anticlockwise,
                                     const std::function<std::pair<T, T>(const Point<T>&)>& dir)
        -> std::vector<Point<T>> {
        if (pointset.size() <= 3) {
            return std::vector<Point<T>>(pointset.begin(), pointset.end());
        }

        const auto min_it = std::min_element(pointset.begin(), pointset.end(),
                                             [&dir](const auto& a, const auto& b) {
                                                 return std::invoke(dir, a) < std::invoke(dir, b);
                                             });
        const auto max_it = std::max_element(pointset.begin(), pointset.end(),
                                             [&dir](const auto& a, const auto& b) {
                                                 return std::invoke(dir, a) < std::invoke(dir, b);
                                             });

        const size_t min_index = static_cast<size_t>(std::distance(pointset.begin(), min_it));
        const size_t max_index = static_cast<size_t>(std::distance(pointset.begin(), max_it));
        const Point<T> min_point = *min_it;

        RDllist rdll(pointset.size());
        auto& v_min = rdll[min_index];
        auto& v_max = rdll[max_index];

        auto process = [&pointset, &dir](Dllink<size_t>* vcurr, Dllink<size_t>* vstop,
                                         const std::function<bool(T, T)>& cmp,
                                         const std::function<bool(T)>& cmp2) {
            while (vcurr != vstop) {
                auto vnext = vcurr->next;
                auto vprev = vcurr->prev;
                const auto& p0 = pointset[vprev->data];
                const auto& p1 = pointset[vcurr->data];
                const auto& p2 = pointset[vnext->data];

                auto dir_p0 = std::invoke(dir, p0);
                auto dir_p1 = std::invoke(dir, p1);
                auto dir_p2 = std::invoke(dir, p2);

                if (cmp(std::get<0>(dir_p1), std::get<0>(dir_p2))
                    || cmp(std::get<0>(dir_p0), std::get<0>(dir_p1))) {
                    T area_diff = (p1.ycoord() - p0.ycoord()) * (p2.xcoord() - p1.xcoord());
                    if (cmp2(area_diff)) {
                        vcurr->detach();
                        vcurr = vprev;
                    } else {
                        vcurr = vnext;
                    }
                } else {
                    vcurr = vnext;
                }
            }
        };

        if (is_anticlockwise) {
            process(&v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a >= 0; });
            process(&v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a >= 0; });
        } else {
            process(&v_min, &v_max, [](T x, T y) { return x >= y; }, [](T a) { return a <= 0; });
            process(&v_max, &v_min, [](T x, T y) { return x <= y; }, [](T a) { return a <= 0; });
        }

        std::vector<Point<T>> result = {min_point};
        for (const auto& v : rdll.from_node(min_index)) {
            result.push_back(pointset[v.data]);
        }
        return result;
    }

    template std::vector<Point<int>> rpolygon_make_monotone_hull<int>(
        std::span<const Point<int>>, bool,
        const std::function<std::pair<int, int>(const Point<int>&)>&);

}  // namespace recti