#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <functional>
#include <span>
#include <vector>

#include "point.hpp"
#include "rdllist.hpp"

namespace recti {
    /**
     * @brief Create a monotone hull from a rectilinear polygon with custom direction
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @param dir Function to extract comparison coordinates
     * @return A vector of points representing the monotone hull
     */
    template <typename T> inline auto rpolygon_make_monotone_hull(
        std::span<const Point<T>> pointset, bool is_anticlockwise,
        const std::function<std::pair<T, T>(const Point<T>&)>& dir) -> std::vector<Point<T>> {
        if (pointset.size() <= 3) {
            return std::vector<Point<T>>(pointset.begin(), pointset.end());
        }

        // Find min and max indices based on the direction function
        const auto min_it
            = std::min_element(pointset.begin(), pointset.end(),
                               [&dir](const auto& a, const auto& b) { return dir(a) < dir(b); });
        const auto max_it
            = std::max_element(pointset.begin(), pointset.end(),
                               [&dir](const auto& a, const auto& b) { return dir(a) < dir(b); });

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

                auto dir_p0 = dir(p0);
                auto dir_p1 = dir(p1);
                auto dir_p2 = dir(p2);

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

    /**
     * @brief Create an x-monotone hull from a rectilinear polygon
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of points representing the x-monotone hull
     */
    template <typename T>
    inline auto rpolygon_make_xmonotone_hull(std::span<const Point<T>> pointset,
                                             bool is_anticlockwise) -> std::vector<Point<T>> {
        return rpolygon_make_monotone_hull<T>(pointset, is_anticlockwise, [](const Point<T>& p) {
            return std::make_pair(p.xcoord(), p.ycoord());
        });
    }

    /**
     * @brief Create a y-monotone hull from a rectilinear polygon
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of points representing the y-monotone hull
     */
    template <typename T>
    inline auto rpolygon_make_ymonotone_hull(std::span<const Point<T>> pointset,
                                             bool is_anticlockwise) -> std::vector<Point<T>> {
        return rpolygon_make_monotone_hull<T>(pointset, is_anticlockwise, [](const Point<T>& p) {
            return std::make_pair(p.ycoord(), p.xcoord());
        });
    }

    /**
     * @brief Create a convex hull from a rectilinear polygon
     *
     * @tparam T The type of the coordinates
     * @param pointset The polygon vertices as points
     * @param is_anticlockwise Whether the polygon is oriented anti-clockwise
     * @return A vector of points representing the convex hull
     */
    template <typename T>
    inline auto rpolygon_make_convex_hull(std::span<const Point<T>> pointset, bool is_anticlockwise)
        -> std::vector<Point<T>> {
        auto xmono_hull = rpolygon_make_xmonotone_hull<T>(pointset, is_anticlockwise);
        return rpolygon_make_ymonotone_hull<T>(xmono_hull, is_anticlockwise);
    }
}  // namespace recti