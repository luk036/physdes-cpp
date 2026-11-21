#include <algorithm>
#include <cstddef>
#include <functional>
#include <recti/rdllist.hpp>
#include <recti/rpolygon.hpp>
#include <recti/rpolygon_hull.hpp>
#include <utility>

namespace recti {
    namespace {
        /**
         * @brief Remove even-index points that lie on a straight boundary edge.
         *
         * With the x-first even-index representation, two consecutive points
         * sharing y make the second point collinear, while sharing x makes the
         * first point collinear. Removing them leaves the polygon unchanged.
         */
        template <typename T>
        auto drop_collinear_points(std::vector<Point<T>> points) -> std::vector<Point<T>> {
            bool changed = true;
            while (changed && points.size() > 2) {
                changed = false;
                for (std::size_t i = 0; i < points.size(); ++i) {
                    const auto j = (i + 1) % points.size();
                    if (points[i].ycoord() == points[j].ycoord()) {
                        points.erase(points.begin() + static_cast<std::ptrdiff_t>(j));
                    } else if (points[i].xcoord() == points[j].xcoord()) {
                        points.erase(points.begin() + static_cast<std::ptrdiff_t>(i));
                    } else {
                        continue;
                    }
                    changed = true;
                    break;
                }
            }
            return points;
        }
    }  // namespace

    /**
     * @brief Create a monotone hull from a rectilinear polygon.
     *
     * Iteratively removes concave vertices from the polygon boundary by
     * walking the circular linked list and detaching points that violate
     * monotonicity or have the wrong turn direction.
     *
     * @tparam T Coordinate type.
     * @param[in] pointset Polygon vertices.
     * @param[in] is_anticlockwise Whether the polygon is oriented anti-clockwise.
     * @param[in] dir Function extracting comparison coordinates from a point.
     * @return A vector of points forming the monotone hull.
     */
    template <typename T> auto rpolygon_make_monotone_hull(
        std::span<const Point<T>> pointset, bool is_anticlockwise,
        const std::function<std::pair<T, T>(const Point<T>&)>& dir) -> std::vector<Point<T>> {
        if (pointset.size() <= 3) {
            return drop_collinear_points(std::vector<Point<T>>(pointset.begin(), pointset.end()));
        }

        const auto min_it = std::min_element(pointset.begin(), pointset.end(),
                                             [&dir](const auto& a, const auto& b) {
                                                 return std::invoke(dir, a) < std::invoke(dir, b);
                                             });
        const auto max_it = std::max_element(pointset.begin(), pointset.end(),
                                             [&dir](const auto& a, const auto& b) {
                                                 return std::invoke(dir, a) < std::invoke(dir, b);
                                             });

        const auto min_index = static_cast<size_t>(std::distance(pointset.begin(), min_it));
        const auto max_index = static_cast<size_t>(std::distance(pointset.begin(), max_it));
        const Point<T> min_point = *min_it;

        RDllist rdll(pointset.size());
        auto& v_min = rdll[min_index];
        auto& v_max = rdll[max_index];

        auto process = [&pointset, &dir](Dllink<size_t>* vcurr, Dllink<size_t>* vstop,
                                         const std::function<bool(T, T)>& cmp,
                                         const std::function<bool(T)>& cmp2) {
            while (vcurr != vstop) {
                auto* vnext = vcurr->next;
                auto* vprev = vcurr->prev;
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
            result.emplace_back(pointset[v.data]);
        }
        return drop_collinear_points(std::move(result));
    }

    template std::vector<Point<int>> rpolygon_make_monotone_hull<int>(
        std::span<const Point<int>>, bool,
        const std::function<std::pair<int, int>(const Point<int>&)>&);

}  // namespace recti